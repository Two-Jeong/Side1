using UnityEngine;
using System;

public static class DebugLog
{
    public static bool isDebugBuild
    {
        get { return UnityEngine.Debug.isDebugBuild; }
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void Log(object message)
    {
        UnityEngine.Debug.Log(message);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void Log(object message, UnityEngine.Object context)
    {
        UnityEngine.Debug.Log(message, context);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void LogError(object message)
    {
        UnityEngine.Debug.LogError(message);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void LogError(object message, UnityEngine.Object context)
    {
        UnityEngine.Debug.LogError(message, context);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void LogWarning(object message)
    {
        UnityEngine.Debug.LogWarning(message.ToString());
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void LogWarning(object message, UnityEngine.Object context)
    {
        UnityEngine.Debug.LogWarning(message.ToString(), context);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void DrawLine(Vector3 start, Vector3 end, Color color = default(Color), float duration = 0.0f, bool depthTest = true)
    {
        UnityEngine.Debug.DrawLine(start, end, color, duration, depthTest);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void DrawRay(Vector3 start, Vector3 dir, Color color = default(Color), float duration = 0.0f, bool depthTest = true)
    {
        UnityEngine.Debug.DrawRay(start, dir, color, duration, depthTest);
    }

    [System.Diagnostics.Conditional("DEV"), System.Diagnostics.Conditional("QA")]
    public static void Assert(bool condition)
    {
        if (!condition) throw new Exception();
    }


    public static void LogEditor(object message)
    {
#if UNITY_EDITOR
            UnityEngine.Debug.Log(message);
#endif
    }

    public static void LogWarningEditor(object message)
    {
#if UNITY_EDITOR
            UnityEngine.Debug.LogWarning(message);
#endif
    }

    public static void LogErrorEditor(object message)
    {
#if UNITY_EDITOR
            UnityEngine.Debug.LogError(message);
#endif
    }
}
