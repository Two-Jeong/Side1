using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Audio;
using Random = UnityEngine.Random;

/// <summary>
/// 게임 내 모든 사운드(BGM, SFX)를 관리하는 매니저
/// </summary>
public class SoundManager : Singleton<SoundManager>
{
    #region Constants
    protected override bool DontDestroyOnload => true;
    
    // 효과음 관련 상수
    private const int MAX_SAME_SFX_COUNT = 3;
    private const float SFX_BLOCK_DURATION = 0.25f;
    private const int DEFAULT_EFFECT_COUNT = 10;
    
    // 오디오 믹서 볼륨 값
    private const float MUTED_VOLUME = -80f;
    private const float NORMAL_VOLUME = 0f;
    
    // 기본 효과음
    private const string BUTTON_SFX = "sfx_common_button";
    #endregion

    #region Enums & Classes
    public enum BGM
    {
        Title,
        Lobby,
        Ingame,
        None = 9999,
    }

    [Serializable]
    public struct ResourceBGM
    {
        public BGM kind;
        public AudioClip[] audioClip;
    }

    [Serializable]
    public class SameTimeInfo
    {
        public string clipName;
        public AudioSource audioSource;
        public float blockTime;
    }
    #endregion

    #region Serialized Fields
    [Header("Audio Settings")]
    [SerializeField] private int effectCount = DEFAULT_EFFECT_COUNT;
    [SerializeField] public string pathSoundEffect = "/06.Sounds/SFX";

    [Header("Mixer")]
    [SerializeField] private AudioMixer audioMixer;
    [SerializeField] private AudioMixerGroup audioMixerGroupEffect;

    [Header("BGM")]
    [SerializeField] private ResourceBGM[] bgmResources;

    [Header("Audio Clips")]
    [SerializeField] public List<AudioClipData> audioClipDatas;
    #endregion

    #region Properties
    public BGM CurrentBGM { get; private set; } = BGM.None;
    public float VolumeBGM { get; private set; }
    public float VolumeSFX { get; private set; }
    public bool IsBGMEnabled => VolumeBGM >= 1f;
    public bool IsSFXEnabled => VolumeSFX >= 1f;
    #endregion

    #region Private Fields
    private AudioSource audioSourceBGM;
    private List<AudioSource> audioSourcesEffect = new List<AudioSource>();
    private GameObject effectSourceContainer;
    private readonly List<SameTimeInfo> activeSFXList = new List<SameTimeInfo>();
    private Coroutine bgmCoroutine;
    #endregion

    #region Unity Lifecycle
    protected override void OnAwake()
    {
        InitializeVolume();
        InitializeAudioSources();
        DebugLog.Log($"SoundManager initialized");
    }

    private void Update()
    {
        UpdateActiveSFXList();
    }
    #endregion

    #region Initialization
    private void InitializeVolume()
    {
        VolumeBGM = PlayerPrefs.GetFloat(Constant.PREFS_BGM, 1f);
        VolumeSFX = PlayerPrefs.GetFloat(Constant.PREFS_SFX, 1f);
        
        ApplyVolumeToMixer("BGM", VolumeBGM);
        ApplyVolumeToMixer("SFX", VolumeSFX);
    }

    private void InitializeAudioSources()
    {
        // BGM 오디오 소스 초기화
        audioSourceBGM = transform.Find("AudioSourceBGM")?.GetComponent<AudioSource>();
        if (audioSourceBGM == null)
        {
            // BGM 오디오 소스가 없으면 생성
            var bgmObject = new GameObject("AudioSourceBGM");
            bgmObject.transform.SetParent(transform);
            audioSourceBGM = bgmObject.AddComponent<AudioSource>();
            audioSourceBGM.playOnAwake = false;
            DebugLog.Log("AudioSourceBGM created dynamically");
        }

        // 효과음 컨테이너 초기화
        effectSourceContainer = transform.Find("AudioSourcesSFX")?.gameObject;
        if (effectSourceContainer == null)
        {
            // 효과음 컨테이너가 없으면 생성
            effectSourceContainer = new GameObject("AudioSourcesSFX");
            effectSourceContainer.transform.SetParent(transform);
            DebugLog.Log("AudioSourcesSFX created dynamically");
        }

        // 효과음 오디오 소스 풀 생성
        CreateEffectAudioSourcePool();
    }

    private void CreateEffectAudioSourcePool()
    {
        audioSourcesEffect = new List<AudioSource>(effectCount);

        for (int i = 0; i < effectCount; i++)
        {
            var audioSource = effectSourceContainer.AddComponent<AudioSource>();
            ConfigureEffectAudioSource(audioSource);
            audioSourcesEffect.Add(audioSource);
        }
    }

    private void ConfigureEffectAudioSource(AudioSource audioSource)
    {
        audioSource.loop = false;
        audioSource.playOnAwake = false;
        audioSource.outputAudioMixerGroup = audioMixerGroupEffect;
        audioSource.Stop();
    }
    #endregion

    #region SFX Management
    private void UpdateActiveSFXList()
    {
        if (activeSFXList.Count == 0) return;

        for (int i = activeSFXList.Count - 1; i >= 0; i--)
        {
            activeSFXList[i].blockTime -= Time.deltaTime;
            if (activeSFXList[i].blockTime <= 0f)
            {
                activeSFXList.RemoveAt(i);
            }
        }
    }

    /// <summary>
    /// 동일한 효과음이 동시에 재생 가능한지 확인
    /// </summary>
    private bool CanPlaySameSFX(AudioClip audioClip)
    {
        if (audioClip == null) return false;
        
        int sameClipCount = activeSFXList.Count(x => string.Equals(x.clipName, audioClip.name));
        return sameClipCount < MAX_SAME_SFX_COUNT;
    }

    /// <summary>
    /// 재생 중인 효과음 목록에 추가
    /// </summary>
    private void RegisterActiveSFX(AudioClip audioClip, AudioSource audioSource)
    {
        if (audioClip == null || audioSource == null) return;
        
        activeSFXList.Add(new SameTimeInfo 
        { 
            clipName = audioClip.name, 
            audioSource = audioSource, 
            blockTime = SFX_BLOCK_DURATION 
        });
    }

    /// <summary>
    /// 사용 가능한 오디오 소스 찾기
    /// </summary>
    private AudioSource GetAvailableEffectSource()
    {
        // 1. 재생 중이지 않은 소스 찾기
        var availableSource = audioSourcesEffect.FirstOrDefault(x => !x.isPlaying);
        if (availableSource != null) return availableSource;

        // 2. 루프하지 않는 소스 찾기
        availableSource = audioSourcesEffect.FirstOrDefault(x => !x.loop);
        if (availableSource != null)
        {
            DebugLog.Log($"No free audio source. Stopping: {availableSource.clip?.name ?? "null"}");
        }

        return availableSource;
    }
    #endregion

    #region Public Methods - Common
    public void PlayButton()
    {
        PlayEffect(BUTTON_SFX);
    }
    #endregion

    #region Volume Management
    /// <summary>
    /// BGM 음소거 상태 토글
    /// </summary>
    public bool ToggleBGMMute()
    {
        VolumeBGM = VolumeBGM >= 1f ? 0f : 1f;
        SaveVolumePreference(Constant.PREFS_BGM, VolumeBGM);
        ApplyVolumeToMixer("BGM", VolumeBGM);
        return IsBGMEnabled;
    }

    /// <summary>
    /// SFX 음소거 상태 토글
    /// </summary>
    public bool ToggleSFXMute()
    {
        VolumeSFX = VolumeSFX >= 1f ? 0f : 1f;
        SaveVolumePreference(Constant.PREFS_SFX, VolumeSFX);
        ApplyVolumeToMixer("SFX", VolumeSFX);
        return IsSFXEnabled;
    }

    private void SaveVolumePreference(string key, float volume)
    {
        PlayerPrefs.SetFloat(key, volume);
        PlayerPrefs.Save();
    }

    private void ApplyVolumeToMixer(string parameterName, float volume)
    {
        if (audioMixer == null) return;
        
        float mixerValue = volume < 1f ? MUTED_VOLUME : NORMAL_VOLUME;
        audioMixer.SetFloat(parameterName, mixerValue);
    }

    // 기존 메서드와의 호환성을 위한 래퍼 메서드
    [Obsolete("Use IsBGMEnabled property instead")]
    public bool GetBGM() => IsBGMEnabled;

    [Obsolete("Use IsSFXEnabled property instead")]
    public bool GetSFX() => IsSFXEnabled;

    [Obsolete("Use ToggleBGMMute() instead")]
    public bool SetMuteBGM() => ToggleBGMMute();

    [Obsolete("Use ToggleSFXMute() instead")]
    public bool SetMuteSFX() => ToggleSFXMute();
    #endregion

    #region BGM Management
    /// <summary>
    /// BGM 정지
    /// </summary>
    /// <param name="fadeDuration">페이드 아웃 시간 (0이면 즉시 정지)</param>
    public void StopBGM(float fadeDuration = 0f)
    {
        if (audioSourceBGM == null) return;

        StopBGMCoroutine();

        if (fadeDuration <= 0f)
        {
            audioSourceBGM.Stop();
            audioSourceBGM.volume = 1f;
        }
        else
        {
            bgmCoroutine = StartCoroutine(FadeOutBGM(fadeDuration));
        }

        CurrentBGM = BGM.None;
    }

    /// <summary>
    /// BGM 재생
    /// </summary>
    /// <param name="bgmType">재생할 BGM 타입</param>
    /// <param name="fadeDuration">페이드 인/아웃 시간</param>
    /// <param name="loop">반복 재생 여부</param>
    public void PlayBGM(BGM bgmType, float fadeDuration = 0f, bool loop = true)
    {
        if (CurrentBGM == bgmType) return;
        if (audioSourceBGM == null) return;

        var clip = GetBGMClip(bgmType);
        if (clip == null) return;

        // 같은 클립이 이미 재생 중이면 무시
        if (audioSourceBGM.isPlaying && audioSourceBGM.clip == clip) return;

        StopBGMCoroutine();
        SetupBGMSource(clip, loop);

        if (fadeDuration > 0f && audioSourceBGM.isPlaying)
        {
            bgmCoroutine = StartCoroutine(CrossFadeBGM(clip, fadeDuration));
        }
        else
        {
            PlayBGMImmediately(clip);
        }

        CurrentBGM = bgmType;
    }

    /// <summary>
    /// BGM 재생 위치 조정
    /// </summary>
    /// <param name="normalizedTime">0~1 사이의 정규화된 시간</param>
    public void SeekBGM(float normalizedTime)
    {
        if (audioSourceBGM?.clip == null) return;
        
        normalizedTime = Mathf.Clamp01(normalizedTime);
        audioSourceBGM.time = audioSourceBGM.clip.length * normalizedTime;
    }

    private AudioClip GetBGMClip(BGM bgmType)
    {
        var resource = bgmResources.FirstOrDefault(x => x.kind == bgmType);
        if (resource.audioClip == null || resource.audioClip.Length == 0) return null;
        
        return resource.audioClip[Random.Range(0, resource.audioClip.Length)];
    }

    private void SetupBGMSource(AudioClip clip, bool loop)
    {
        audioSourceBGM.time = 0f;
        audioSourceBGM.loop = loop;
    }

    private void PlayBGMImmediately(AudioClip clip)
    {
        audioSourceBGM.volume = 1f;
        audioSourceBGM.clip = clip;
        audioSourceBGM.Play();
    }

    private void StopBGMCoroutine()
    {
        if (bgmCoroutine != null)
        {
            StopCoroutine(bgmCoroutine);
            bgmCoroutine = null;
        }
    }

    private IEnumerator FadeOutBGM(float duration)
    {
        float startVolume = audioSourceBGM.volume;
        float elapsed = 0f;

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            audioSourceBGM.volume = Mathf.Lerp(startVolume, 0f, elapsed / duration);
            yield return null;
        }

        audioSourceBGM.Stop();
        audioSourceBGM.volume = 1f;
        bgmCoroutine = null;
    }

    private IEnumerator CrossFadeBGM(AudioClip newClip, float duration)
    {
        // Fade out
        float halfDuration = duration * 0.5f;
        yield return FadeOutBGM(halfDuration);

        // Switch clip
        audioSourceBGM.clip = newClip;
        audioSourceBGM.Play();

        // Fade in
        float elapsed = 0f;
        while (elapsed < halfDuration)
        {
            elapsed += Time.deltaTime;
            audioSourceBGM.volume = Mathf.Lerp(0f, 1f, elapsed / halfDuration);
            yield return null;
        }

        audioSourceBGM.volume = 1f;
        bgmCoroutine = null;
    }

    // 기존 메서드와의 호환성을 위한 래퍼 메서드
    [Obsolete("Use SeekBGM() instead")]
    public void WindBGM(float time) => SeekBGM(time);

    #endregion

    #region SFX Playback
    /// <summary>
    /// 특정 효과음이 재생 중인지 확인
    /// </summary>
    public bool IsEffectPlaying(string effectId)
    {
        if (string.IsNullOrEmpty(effectId)) return false;

        var clipData = GetAudioClipData(effectId);
        if (clipData?.clip == null) return false;

        return audioSourcesEffect.Any(x => x.clip == clipData.clip && x.isPlaying);
    }

    /// <summary>
    /// 효과음 재생 (AudioClip 직접 전달)
    /// </summary>
    public void PlayEffect(AudioClip clip)
    {
        if (clip == null) return;
        PlayEffect(clip.name);
    }

    /// <summary>
    /// 효과음 재생 (ID로 재생)
    /// </summary>
    public void PlayEffect(string effectId, bool loop = false)
    {
        if (string.IsNullOrEmpty(effectId)) return;

        var clipData = GetAudioClipData(effectId);
        if (clipData?.clip == null)
        {
            DebugLog.LogWarning($"Audio clip not found: {effectId}");
            return;
        }

        if (!CanPlaySameSFX(clipData.clip)) return;

        var audioSource = GetAvailableEffectSource();
        if (audioSource == null)
        {
            DebugLog.LogWarning("No available audio source for effect");
            return;
        }

        PlayEffectOnSource(audioSource, clipData.clip, loop);
        RegisterActiveSFX(clipData.clip, audioSource);
    }

    /// <summary>
    /// 특정 효과음 정지
    /// </summary>
    public void StopEffect(string effectId)
    {
        if (string.IsNullOrEmpty(effectId)) return;

        var clipData = GetAudioClipData(effectId);
        if (clipData?.clip == null) return;

        var playingSources = audioSourcesEffect
            .Where(x => x.clip == clipData.clip && x.isPlaying)
            .ToList();

        foreach (var source in playingSources)
        {
            source.Stop();
        }
    }

    /// <summary>
    /// 모든 효과음 정지
    /// </summary>
    public void StopAllEffects()
    {
        foreach (var source in audioSourcesEffect)
        {
            if (source.isPlaying)
            {
                source.Stop();
            }
        }
        
        activeSFXList.Clear();
    }

    private AudioClipData GetAudioClipData(string effectId)
    {
        return audioClipDatas?.FirstOrDefault(x => x.id == effectId);
    }

    private void PlayEffectOnSource(AudioSource source, AudioClip clip, bool loop)
    {
        source.outputAudioMixerGroup = audioMixerGroupEffect;
        source.clip = clip;
        source.loop = loop;
        source.Play();
        
        // 리스트 끝으로 이동 (LRU 방식)
        audioSourcesEffect.Remove(source);
        audioSourcesEffect.Add(source);
    }

    // 기존 메서드와의 호환성을 위한 래퍼 메서드
    [Obsolete("Use IsEffectPlaying() instead")]
    public bool IsPlayEffect(string id) => IsEffectPlaying(id);

    [Obsolete("Use StopAllEffects() instead")]
    public void StopAllEffect() => StopAllEffects();
    #endregion

    #region Cleanup
    private void OnDestroy()
    {
        StopBGMCoroutine();
        StopAllEffects();
    }
    #endregion
}

/// <summary>
/// 오디오 클립 데이터
/// </summary>
[Serializable]
public class AudioClipData
{
    public string id;
    public AudioClip clip;
}
