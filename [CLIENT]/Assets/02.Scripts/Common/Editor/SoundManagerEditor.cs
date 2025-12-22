using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using UnityEditor;
using UnityEngine;

/// <summary>
/// SoundManager의 커스텀 에디터
/// </summary>
[CustomEditor(typeof(SoundManager))]
public class SoundManagerEditor : Editor
{
    #region Constants
    private const float BUTTON_HEIGHT = 40f;
    private const string LOAD_BUTTON_TEXT = "Load Audio Clips";
    private const string CLEAR_BUTTON_TEXT = "Clear Audio Clips";
    private const string REFRESH_BUTTON_TEXT = "Refresh Audio Clips";
    
    // 지원하는 오디오 파일 확장자
    private static readonly string[] SUPPORTED_AUDIO_EXTENSIONS = { "*.mp3", "*.wav", "*.ogg" };
    #endregion

    #region Private Fields
    private SoundManager soundManager;
    private SerializedProperty audioClipDatasProperty;
    private SerializedProperty pathSoundEffectProperty;
    
    // 에디터 상태
    private bool showAudioClipList = true;
    private string searchFilter = "";
    private int loadedClipsCount = 0;
    #endregion

    #region Unity Lifecycle
    private void OnEnable()
    {
        soundManager = target as SoundManager;
        
        // SerializedProperty 캐싱
        audioClipDatasProperty = serializedObject.FindProperty("audioClipDatas");
        pathSoundEffectProperty = serializedObject.FindProperty("pathSoundEffect");
    }
    #endregion

    #region Inspector GUI
    public override void OnInspectorGUI()
    {
        serializedObject.Update();
        
        DrawHeader();
        EditorGUILayout.Space(10);
        
        DrawLoadSection();
        EditorGUILayout.Space(10);
        
        DrawAudioClipListSection();
        EditorGUILayout.Space(10);
        
        DrawDefaultInspector();
        
        serializedObject.ApplyModifiedProperties();
    }

    /// <summary>
    /// 헤더 영역 그리기
    /// </summary>
    private void DrawHeader()
    {
        EditorGUILayout.BeginVertical("box");
        
        EditorGUILayout.LabelField("Sound Manager Editor", EditorStyles.boldLabel);
        
        if (soundManager != null && soundManager.audioClipDatas != null)
        {
            EditorGUILayout.LabelField($"Loaded Audio Clips: {soundManager.audioClipDatas.Count}", EditorStyles.miniLabel);
        }
        
        EditorGUILayout.EndVertical();
    }

    /// <summary>
    /// 오디오 클립 로드 섹션 그리기
    /// </summary>
    private void DrawLoadSection()
    {
        EditorGUILayout.BeginVertical("box");
        EditorGUILayout.LabelField("Audio Clip Management", EditorStyles.boldLabel);
        
        // 경로 표시 및 수정
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Path:", GUILayout.Width(50));
        
        EditorGUI.BeginChangeCheck();
        string newPath = EditorGUILayout.TextField(soundManager.pathSoundEffect);
        if (EditorGUI.EndChangeCheck())
        {
            Undo.RecordObject(soundManager, "Change Sound Effect Path");
            soundManager.pathSoundEffect = newPath;
            EditorUtility.SetDirty(soundManager);
        }
        EditorGUILayout.EndHorizontal();
        
        // 버튼들
        EditorGUILayout.BeginHorizontal();
        
        GUI.backgroundColor = Color.green;
        if (GUILayout.Button(LOAD_BUTTON_TEXT, GUILayout.Height(BUTTON_HEIGHT)))
        {
            LoadAudioClips();
        }
        
        GUI.backgroundColor = Color.yellow;
        if (GUILayout.Button(REFRESH_BUTTON_TEXT, GUILayout.Height(BUTTON_HEIGHT)))
        {
            RefreshAudioClips();
        }
        
        GUI.backgroundColor = Color.red;
        if (GUILayout.Button(CLEAR_BUTTON_TEXT, GUILayout.Height(BUTTON_HEIGHT)))
        {
            ClearAudioClips();
        }
        
        GUI.backgroundColor = Color.white;
        EditorGUILayout.EndHorizontal();
        
        // 로드 결과 표시
        if (loadedClipsCount > 0)
        {
            EditorGUILayout.HelpBox($"Successfully loaded {loadedClipsCount} audio clips!", MessageType.Info);
        }
        
        EditorGUILayout.EndVertical();
    }

    /// <summary>
    /// 오디오 클립 리스트 섹션 그리기
    /// </summary>
    private void DrawAudioClipListSection()
    {
        if (soundManager.audioClipDatas == null || soundManager.audioClipDatas.Count == 0)
            return;
        
        EditorGUILayout.BeginVertical("box");
        
        // 헤더와 토글
        EditorGUILayout.BeginHorizontal();
        showAudioClipList = EditorGUILayout.Foldout(showAudioClipList, "Audio Clips List", true);
        EditorGUILayout.LabelField($"({soundManager.audioClipDatas.Count} clips)", EditorStyles.miniLabel);
        EditorGUILayout.EndHorizontal();
        
        if (showAudioClipList)
        {
            // 검색 필터
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Search:", GUILayout.Width(50));
            searchFilter = EditorGUILayout.TextField(searchFilter);
            if (GUILayout.Button("Clear", GUILayout.Width(50)))
            {
                searchFilter = "";
                GUI.FocusControl(null);
            }
            EditorGUILayout.EndHorizontal();
            
            EditorGUILayout.Space(5);
            
            // 오디오 클립 리스트 표시
            DrawAudioClipsList();
        }
        
        EditorGUILayout.EndVertical();
    }

    /// <summary>
    /// 오디오 클립 리스트 그리기
    /// </summary>
    private void DrawAudioClipsList()
    {
        var filteredClips = string.IsNullOrEmpty(searchFilter) 
            ? soundManager.audioClipDatas 
            : soundManager.audioClipDatas.Where(x => 
                x.id.ToLower().Contains(searchFilter.ToLower()) || 
                (x.clip != null && x.clip.name.ToLower().Contains(searchFilter.ToLower()))
            ).ToList();

        EditorGUILayout.BeginVertical();
        
        // 헤더
        EditorGUILayout.BeginHorizontal("box");
        EditorGUILayout.LabelField("ID", EditorStyles.boldLabel, GUILayout.Width(200));
        EditorGUILayout.LabelField("Audio Clip", EditorStyles.boldLabel);
        EditorGUILayout.LabelField("Duration", EditorStyles.boldLabel, GUILayout.Width(80));
        EditorGUILayout.EndHorizontal();
        
        // 스크롤 뷰
        var scrollViewStyle = new GUIStyle(GUI.skin.scrollView) { padding = new RectOffset(5, 5, 5, 5) };
        using (var scrollView = new EditorGUILayout.ScrollViewScope(Vector2.zero, GUILayout.MaxHeight(300)))
        {
            foreach (var clipData in filteredClips)
            {
                DrawAudioClipItem(clipData);
            }
        }
        
        EditorGUILayout.EndVertical();
    }

    /// <summary>
    /// 개별 오디오 클립 아이템 그리기
    /// </summary>
    private void DrawAudioClipItem(AudioClipData clipData)
    {
        EditorGUILayout.BeginHorizontal("box");
        
        // ID
        EditorGUILayout.LabelField(clipData.id, GUILayout.Width(200));
        
        // Audio Clip
        EditorGUI.BeginChangeCheck();
        var newClip = EditorGUILayout.ObjectField(clipData.clip, typeof(AudioClip), false) as AudioClip;
        if (EditorGUI.EndChangeCheck())
        {
            Undo.RecordObject(soundManager, "Change Audio Clip");
            clipData.clip = newClip;
            if (newClip != null && string.IsNullOrEmpty(clipData.id))
            {
                clipData.id = newClip.name;
            }
            EditorUtility.SetDirty(soundManager);
        }
        
        // Duration
        if (clipData.clip != null)
        {
            EditorGUILayout.LabelField($"{clipData.clip.length:F2}s", GUILayout.Width(80));
        }
        else
        {
            EditorGUILayout.LabelField("-", GUILayout.Width(80));
        }
        
        EditorGUILayout.EndHorizontal();
    }

    /// <summary>
    /// 기본 인스펙터 그리기 (나머지 필드들)
    /// </summary>
    private void DrawDefaultInspector()
    {
        EditorGUILayout.BeginVertical("box");
        EditorGUILayout.LabelField("Other Settings", EditorStyles.boldLabel);
        
        // audioClipDatas와 pathSoundEffect를 제외한 나머지 프로퍼티 그리기
        SerializedProperty prop = serializedObject.GetIterator();
        bool enterChildren = true;
        
        while (prop.NextVisible(enterChildren))
        {
            enterChildren = false;
            
            // 이미 처리한 프로퍼티는 건너뛰기
            if (prop.name == "m_Script" || 
                prop.name == "audioClipDatas" || 
                prop.name == "pathSoundEffect")
                continue;
            
            EditorGUILayout.PropertyField(prop, true);
        }
        
        EditorGUILayout.EndVertical();
    }
    #endregion

    #region Audio Clip Management
    /// <summary>
    /// 오디오 클립 로드
    /// </summary>
    private void LoadAudioClips()
    {
        if (soundManager == null) return;
        
        Undo.RecordObject(soundManager, "Load Audio Clips");
        
        soundManager.audioClipDatas = new List<AudioClipData>();
        loadedClipsCount = 0;
        
        foreach (string pattern in SUPPORTED_AUDIO_EXTENSIONS)
        {
            LoadAudioClipsWithPattern(pattern);
        }
        
        // 중복 제거
        RemoveDuplicates();
        
        // 알파벳 순으로 정렬
        soundManager.audioClipDatas = soundManager.audioClipDatas
            .OrderBy(x => x.id)
            .ToList();
        
        Debug.Log($"[SoundManager] Loaded {loadedClipsCount} audio clips from {soundManager.pathSoundEffect}");
        EditorUtility.SetDirty(soundManager);
    }

    /// <summary>
    /// 오디오 클립 새로고침 (기존 것 유지하면서 새로운 것만 추가)
    /// </summary>
    private void RefreshAudioClips()
    {
        if (soundManager == null) return;
        
        Undo.RecordObject(soundManager, "Refresh Audio Clips");
        
        if (soundManager.audioClipDatas == null)
        {
            soundManager.audioClipDatas = new List<AudioClipData>();
        }
        
        var existingIds = new HashSet<string>(soundManager.audioClipDatas.Select(x => x.id));
        int newClipsCount = 0;
        
        foreach (string pattern in SUPPORTED_AUDIO_EXTENSIONS)
        {
            newClipsCount += RefreshAudioClipsWithPattern(pattern, existingIds);
        }
        
        // 중복 제거
        RemoveDuplicates();
        
        // 알파벳 순으로 정렬
        soundManager.audioClipDatas = soundManager.audioClipDatas
            .OrderBy(x => x.id)
            .ToList();
        
        Debug.Log($"[SoundManager] Added {newClipsCount} new audio clips");
        EditorUtility.SetDirty(soundManager);
    }

    /// <summary>
    /// 오디오 클립 모두 제거
    /// </summary>
    private void ClearAudioClips()
    {
        if (soundManager == null) return;
        
        if (EditorUtility.DisplayDialog("Clear Audio Clips", 
            "Are you sure you want to clear all audio clips?", 
            "Yes", "No"))
        {
            Undo.RecordObject(soundManager, "Clear Audio Clips");
            soundManager.audioClipDatas = new List<AudioClipData>();
            loadedClipsCount = 0;
            Debug.Log("[SoundManager] Cleared all audio clips");
            EditorUtility.SetDirty(soundManager);
        }
    }

    /// <summary>
    /// 특정 패턴의 오디오 파일 로드
    /// </summary>
    private void LoadAudioClipsWithPattern(string pattern)
    {
        string fullPath = Application.dataPath + soundManager.pathSoundEffect;
        
        if (!Directory.Exists(fullPath))
        {
            Debug.LogWarning($"[SoundManager] Directory not found: {fullPath}");
            return;
        }
        
        string[] filePaths = Directory.GetFiles(fullPath, pattern, SearchOption.AllDirectories);
        
        foreach (string filePath in filePaths)
        {
            AudioClipData clipData = CreateAudioClipData(filePath);
            if (clipData != null)
            {
                soundManager.audioClipDatas.Add(clipData);
                loadedClipsCount++;
            }
        }
    }

    /// <summary>
    /// 특정 패턴의 새로운 오디오 파일만 추가
    /// </summary>
    private int RefreshAudioClipsWithPattern(string pattern, HashSet<string> existingIds)
    {
        string fullPath = Application.dataPath + soundManager.pathSoundEffect;
        
        if (!Directory.Exists(fullPath))
        {
            Debug.LogWarning($"[SoundManager] Directory not found: {fullPath}");
            return 0;
        }
        
        string[] filePaths = Directory.GetFiles(fullPath, pattern, SearchOption.AllDirectories);
        int addedCount = 0;
        
        foreach (string filePath in filePaths)
        {
            AudioClipData clipData = CreateAudioClipData(filePath);
            if (clipData != null && !existingIds.Contains(clipData.id))
            {
                soundManager.audioClipDatas.Add(clipData);
                existingIds.Add(clipData.id);
                addedCount++;
            }
        }
        
        return addedCount;
    }

    /// <summary>
    /// 파일 경로로부터 AudioClipData 생성
    /// </summary>
    private AudioClipData CreateAudioClipData(string filePath)
    {
        // Assets으로 시작하는 경로 추출
        int assetsIndex = filePath.IndexOf("Assets");
        if (assetsIndex < 0)
        {
            Debug.LogWarning($"[SoundManager] Invalid file path: {filePath}");
            return null;
        }
        
        string assetPath = filePath.Substring(assetsIndex).Replace('\\', '/');
        AudioClip clip = AssetDatabase.LoadAssetAtPath<AudioClip>(assetPath);
        
        if (clip == null)
        {
            Debug.LogWarning($"[SoundManager] Failed to load audio clip: {assetPath}");
            return null;
        }
        
        return new AudioClipData
        {
            id = clip.name,
            clip = clip
        };
    }

    /// <summary>
    /// 중복 제거
    /// </summary>
    private void RemoveDuplicates()
    {
        if (soundManager.audioClipDatas == null || soundManager.audioClipDatas.Count <= 1)
            return;
        
        var uniqueClips = new Dictionary<string, AudioClipData>();
        
        foreach (var clipData in soundManager.audioClipDatas)
        {
            if (!uniqueClips.ContainsKey(clipData.id))
            {
                uniqueClips[clipData.id] = clipData;
            }
            else
            {
                Debug.LogWarning($"[SoundManager] Duplicate audio clip ID found: {clipData.id}");
            }
        }
        
        soundManager.audioClipDatas = uniqueClips.Values.ToList();
    }
    #endregion

    #region Utility Methods
    /// <summary>
    /// 오디오 클립 ID 목록을 열거형 형식으로 생성 (디버그용)
    /// </summary>
    private string GenerateEnumString()
    {
        if (soundManager.audioClipDatas == null || soundManager.audioClipDatas.Count == 0)
            return "// No audio clips loaded";
        
        var sb = new StringBuilder();
        sb.AppendLine("// Generated Audio Clip IDs");
        sb.AppendLine("public enum AudioClipID");
        sb.AppendLine("{");
        
        foreach (var clipData in soundManager.audioClipDatas)
        {
            // 열거형에 사용 가능한 형식으로 변환
            string enumName = clipData.id
                .Replace(" ", "_")
                .Replace("-", "_")
                .Replace(".", "_");
            
            sb.AppendLine($"    {enumName},");
        }
        
        sb.AppendLine("}");
        
        return sb.ToString();
    }

    /// <summary>
    /// 오디오 클립 통계 정보 출력
    /// </summary>
    [MenuItem("Tools/Sound Manager/Show Audio Clip Statistics")]
    public static void ShowAudioClipStatistics()
    {
        var soundManager = FindObjectOfType<SoundManager>();
        
        if (soundManager == null)
        {
            Debug.LogWarning("[SoundManager] No SoundManager found in the scene");
            return;
        }
        
        if (soundManager.audioClipDatas == null || soundManager.audioClipDatas.Count == 0)
        {
            Debug.Log("[SoundManager] No audio clips loaded");
            return;
        }
        
        float totalDuration = 0f;
        long totalSize = 0;
        int missingClips = 0;
        
        foreach (var clipData in soundManager.audioClipDatas)
        {
            if (clipData.clip != null)
            {
                totalDuration += clipData.clip.length;
                // 대략적인 크기 계산 (샘플 * 채널 * 바이트)
                totalSize += (long)(clipData.clip.samples * clipData.clip.channels * 2);
            }
            else
            {
                missingClips++;
            }
        }
        
        Debug.Log($"===== Audio Clip Statistics =====");
        Debug.Log($"Total Clips: {soundManager.audioClipDatas.Count}");
        Debug.Log($"Missing Clips: {missingClips}");
        Debug.Log($"Total Duration: {totalDuration:F2} seconds ({totalDuration / 60:F2} minutes)");
        Debug.Log($"Estimated Size: {totalSize / (1024f * 1024f):F2} MB");
        Debug.Log($"================================");
    }
    #endregion
}
