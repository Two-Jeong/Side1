using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;

[CustomEditor(typeof(SoundManager))]
public class SoundManagerEditor : Editor
{
    SoundManager instnace;

    private void OnEnable()
    {
        instnace = target as SoundManager;
    }

    public override void OnInspectorGUI()
    {
        if (GUILayout.Button("Load AudioClips", GUILayout.Height(40f)))
        {
            instnace.audioClipDatas = new List<AudioClipData>();
            Load("*.mp3");
            Load("*.wav");
            Load("*.ogg");
            DebugLog.Log("Load End");
            EditorUtility.SetDirty(instnace);
        }

        base.OnInspectorGUI();
    }

    public void Load(string _pattern)
    {
        string[] filePahts = Directory.GetFiles(Application.dataPath + instnace.pathSoundEffect, _pattern, SearchOption.AllDirectories);

        string enumString = "";

        for (int i = 0; i < filePahts.Length; i++)
        {
            int tempIndex = filePahts[i].IndexOf("Assets");  // Assets으로 시작하는 인덱스 찾기     DebugLog.Log($"filePahts[{i}] = {filePahts[i]}");            
            string tempPath = filePahts[i].Substring(tempIndex); // Assets으로 시작하는 주소   DebugLog.Log($"Substring = {filePahts[i].Substring(tempIndex)}");

            AudioClipData tempData = new AudioClipData();
            tempData.clip = AssetDatabase.LoadAssetAtPath<AudioClip>(tempPath);

            tempData.id = tempData.clip.name;
            instnace.audioClipDatas.Add(tempData);

            enumString += "\n" + AssetDatabase.LoadAssetAtPath<AudioClip>(tempPath).name + ",";
        }
        DebugLog.Log(enumString);
    }
}
