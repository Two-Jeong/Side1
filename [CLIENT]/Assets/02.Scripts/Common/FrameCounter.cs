using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary> FPS 표기 /summary>
public class FrameCounter : MonoBehaviour
{
#if UNITY_EDITOR
    private float deltaTime = 0f;
    private float timer = 0;
    
    [SerializeField] private int size = 30;
    [SerializeField] private Color color = Color.red;

    void Update()
    {
        deltaTime += (Time.unscaledDeltaTime - deltaTime) * 0.1f;
        timer += Time.deltaTime;
    }

    private void OnGUI()
    {
        float ms = deltaTime * 1000f;
        float fps = 1.0f / deltaTime;
        string text = string.Format("{0:0.} FPS ({1:0.0} ms)", fps, ms);
        DrawGUI(text, 30);
    }

    public void DrawGUI(string text, float y)
    {
        GUIStyle style = new GUIStyle();

        Rect rect = new Rect(30, y, Screen.width, Screen.height);
        style.alignment = TextAnchor.UpperLeft;
        style.fontSize = size;
        style.normal.textColor = color;
        style.fontStyle = FontStyle.Bold;

        GUI.Label(rect, text, style);
    }
#endif
}
