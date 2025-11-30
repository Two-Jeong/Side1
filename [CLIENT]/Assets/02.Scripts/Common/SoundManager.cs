using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Audio;

using Random = UnityEngine.Random;

public class SoundManager : Singleton<SoundManager>
{
    protected override bool DontDestroyOnload => true;
    private readonly int COUNT_SFX_SAME_TIME = 3;
    private readonly float TIME_SFX_SAME_TIME = 0.25f;

    public enum BGM
    {
        Title,
        Lobby,
        Ingame,
        None = 9999,
    }

    [System.Serializable]
    public struct ResourceBGM
    {
        public BGM kind;
        public AudioClip[] audioClip;
    }

    [System.Serializable]
    public class SameTimeInfo
    {
        public string clipName;
        public AudioSource audioSource;
        public float blockTime;
    }

    [SerializeField] private int effectCount = 10;

    public string pathSoundEffect = "/06.Sounds/SFX";

    [Header("Mixer")]
    public AudioMixer audioMixer;
    public AudioMixerGroup audioMixerGroupEffect;

    [Header("BGM")]
    public ResourceBGM[] bgmResources;

    public BGM CurrentBGM => currentBGM;
    private BGM currentBGM = BGM.None;

    private float volumeBGM = 0f;
    public float VolumeBGM { get { return volumeBGM; } }
    private float volumeSFX = 0f;
    public float VolumeSFX { get { return volumeSFX; } }

    public List<AudioClipData> audioClipDatas;

    private List<AudioSource> audioSourcesEffect;
    private AudioSource audioSourceBGM;
    private GameObject objEffect;

    /// <summary> 재생 중인 효과음 리스트 </summary>
    private List<SameTimeInfo> sfxPlayingList = new List<SameTimeInfo>();

    protected override void OnAwake()
    {
        volumeBGM = PlayerPrefs.GetFloat(Constant.PREFS_BGM, 1f);
        volumeSFX = PlayerPrefs.GetFloat(Constant.PREFS_SFX, 1f);

        if (volumeBGM < 1f)
            audioMixer.SetFloat("BGM", -80f);
        else
            audioMixer.SetFloat("BGM", 0f);

        if (volumeSFX < 1f)
            audioMixer.SetFloat("SFX", -80f);
        else
            audioMixer.SetFloat("SFX", 0f);

        audioSourceBGM = transform.Find("AudioSourceBGM").GetComponent<AudioSource>();
        objEffect = transform.Find("AudioSourcesSFX").gameObject;

        audioSourcesEffect = new List<AudioSource>();

        for (int i = 0; i < effectCount; i++)
        {
            AudioSource tempAudio = objEffect.AddComponent<AudioSource>();
            tempAudio.loop = false;
            tempAudio.playOnAwake = false;
            //tempAudio.outputAudioMixerGroup = audioMixerGroupEffect;
            tempAudio.Stop();
            audioSourcesEffect.Add(tempAudio);
        }

        DebugLog.Log($"SoundManager. OnAwake");
    }

    private void Update()
    {
        UpdateSFX();
    }

    private void UpdateSFX()
    {
        if (sfxPlayingList.Count == 0)
            return;

        sfxPlayingList.ForEach(x => x.blockTime -= Time.deltaTime);
        sfxPlayingList.RemoveAll(x => x.blockTime <= 0f);
    }

    /// <summary> 동시에 재생 가능한 효과음 체크 </summary>
    private bool AddablePlaySameSFXList(AudioClip _audioClip)
    {
        return sfxPlayingList.Count(x => string.Equals(x.clipName, _audioClip.name)) < COUNT_SFX_SAME_TIME;
    }

    /// <summary> 동시에 재생 가능한 효과음 추가 </summary>
    private void AddPlaySameSFXList(AudioClip _audioClip, AudioSource _audioSource)
    {
        sfxPlayingList.Add(new SameTimeInfo() { clipName = _audioClip.name, audioSource = _audioSource, blockTime = TIME_SFX_SAME_TIME });
    }

    public void PlayButton()
    {
        PlayEffect("sfx_common_button");
    }

    #region Volume

    public bool GetBGM()
    {
        return volumeBGM == 1;
    }

    public bool GetSFX()
    {
        return volumeSFX == 1;
    }

    public bool SetMuteBGM()
    {
        volumeBGM = volumeBGM == 1f ? 0f : 1f;

        PlayerPrefs.SetFloat(Constant.PREFS_BGM, volumeBGM);

        if (volumeBGM < 1f)
            audioMixer.SetFloat("BGM", -80f);
        else
            audioMixer.SetFloat("BGM", 0f);

        return volumeBGM == 1;
    }

    public bool SetMuteSFX()
    {
        volumeSFX = volumeSFX == 1f ? 0f : 1f;

        PlayerPrefs.SetFloat(Constant.PREFS_SFX, volumeSFX);

        if (volumeSFX < 1f)
            audioMixer.SetFloat("SFX", -80f);
        else
            audioMixer.SetFloat("SFX", 0f);

        return volumeSFX == 1;
    }
    #endregion

    public void StopBGM(float _duration = 0f)
    {
        if (audioSourceBGM == null)
            return;

        StopAllCoroutines();

        if (_duration == 0f)
        {
            audioSourceBGM.Stop();
        }
        else
            StartCoroutine(DoStopBGM(_duration));

        currentBGM = BGM.None;
    }

    private IEnumerator DoStopBGM(float _duration)
    {
        while (audioSourceBGM.volume > 0f)
        {
            audioSourceBGM.volume -= (1 / _duration) * Time.deltaTime;
            yield return null;
        }

        audioSourceBGM.Stop();
    }

    public void PlayBGM(BGM _kind, float _duraion = 0f, bool _loop = true)
    {
        if (CurrentBGM == _kind)
            return;

        StopAllCoroutines();

        AudioClip tempClip = bgmResources.Any(x => x.kind == _kind) ? bgmResources.First(x => x.kind == _kind).audioClip[Random.Range(0, bgmResources.First(x => x.kind == _kind).audioClip.Length)] : null;

        if (audioSourceBGM.isPlaying && audioSourceBGM.clip == tempClip)
            return;

        audioSourceBGM.time = 0f;
        audioSourceBGM.loop = _loop;

        if (_duraion > 0f && audioSourceBGM.isPlaying)
        {
            StartCoroutine(DOSwitchBGM(tempClip, _duraion));
        }
        else
        {
            audioSourceBGM.volume = 1f;
            audioSourceBGM.clip = tempClip;
            audioSourceBGM.Play();
        }

        currentBGM = _kind;
    }

    public void WindBGM(float _time)
    {
        audioSourceBGM.time = audioSourceBGM.clip.length * _time;
    }

    private IEnumerator DOSwitchBGM(AudioClip _clip, float _duration)
    {
        while (audioSourceBGM.volume > 0f)
        {
            audioSourceBGM.volume -= (1 / _duration) * Time.deltaTime;
            yield return null;
        }

        audioSourceBGM.clip = _clip;
        audioSourceBGM.Play();

        while (audioSourceBGM.volume < 1f)
        {
            audioSourceBGM.volume += (1 / _duration) * Time.deltaTime;
            yield return null;
        }
    }

    public bool IsPlayEffect(string _id)
    {
        if (string.IsNullOrEmpty(_id))
            return false;

        AudioClipData tempData = audioClipDatas.Find(x => x.id == _id);
        List<AudioSource> tempSource = audioSourcesEffect.FindAll(x => x.clip == tempData.clip && x.isPlaying);

        if (tempSource.Count > 0)
            return true;
        else
            return false;
    }

    public void PlayEffect(AudioClip _clip)
    {
        PlayEffect(_clip.name);
    }

    public void PlayEffect(string _id, bool _loop = false)
    {
        if (string.IsNullOrEmpty(_id))
            return;

        AudioClipData tempData = audioClipDatas.Find(x => x.id == _id);
        if (tempData != null)
        {
            if (AddablePlaySameSFXList(tempData.clip))
            {
                AudioSource tempSource = audioSourcesEffect.Find(x => x.isPlaying == false);

                if (tempSource == null)
                {
                    tempSource = audioSourcesEffect.Find(x => x.loop == false);
                    if (tempSource != null)
                    {
                        string beforeClip = tempSource.clip != null ? tempSource.clip.name : string.Empty;
                        //DebugLog.Log($"빈 오디오 소스가 없어서 기존 오디오를 멈추고 출력합니다. ({tempSource.clip.name} -> {_id})");
                    }
                    //else
                    //    DebugLog.Log($"빈 오디오 소스가 없습니다.");
                }

                if (tempSource == null)
                    return;

                tempSource.outputAudioMixerGroup = audioMixerGroupEffect;
                tempSource.clip = tempData.clip;
                tempSource.loop = _loop;
                tempSource.Play();
                audioSourcesEffect.Remove(tempSource);
                audioSourcesEffect.Add(tempSource);

                AddPlaySameSFXList(tempData.clip, tempSource);
            }
        }
        else
            DebugLog.Log($"해당 오디오 클립이 없습니다. ({_id})");
    }

    public void StopEffect(string _id)
    {
        AudioClipData tempData = audioClipDatas.Find(x => x.id == _id);
        List<AudioSource> tempSource = audioSourcesEffect.FindAll(x => x.clip == tempData.clip && x.isPlaying);
        if (tempSource.Count > 0)
        {
            foreach (var s in tempSource)
                s.Stop();
        }
    }

    public void StopAllEffect()
    {
        for (int i = 0; i < audioSourcesEffect.Count; i++)
        {
            if (audioSourcesEffect[i].isPlaying)
                audioSourcesEffect[i].Stop();
        }
    }
}

[System.Serializable]
public class AudioClipData
{
    public string id;
    public AudioClip clip;
}