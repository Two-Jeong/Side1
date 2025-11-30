using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;

public static class Util
{
    public static string GetGameVersion()
    {
#if DEV
        return "Ver " + Application.version + "d" + $" ({GameDataManager.VersionQA.ToString()})";
#elif QA
            return "Ver " + Application.version + "q" + $" ({GameDataManager.VersionQA.ToString()})";
#else
        return "Ver " + Application.version;
#endif
    }

    public static T GetOrAddComponent<T>(GameObject go) where T : UnityEngine.Component
    {
        T component = go.GetComponent<T>();
        if (component == null)
            component = go.AddComponent<T>();
        return component;
    }

    /// <summary> HtmlString을 컬러로 반환 </summary>
    public static Color TryParseColor(string _htmlStr)
    {
        ColorUtility.TryParseHtmlString(_htmlStr, out Color color);
        return color;
    }

    /// <summary> 컬러를 HtmlString으로 반환 </summary>
    public static string ParseHtmlString(Color _color)
    {
        return ColorUtility.ToHtmlStringRGB(_color);
    }

    public static T StringToEnum<T>(string s)
    {
        if (Enum.TryParse(typeof(T), s, out object result))
            return (T)result;
        return default;
    }

    public static void LayoutUpdate(RectTransform rect)
    {
        RectTransform[] rectArr = rect.GetComponentsInChildren<RectTransform>();
        for (int i = 0; i < rectArr.Length; i++)
        {
            LayoutRebuilder.ForceRebuildLayoutImmediate(rectArr[i]);
        }
    }

    public static List<T> ShuffleList<T>(List<T> values)
    {
        System.Random rand = new System.Random();
        var shuffled = values.OrderBy(_ => rand.Next()).ToList();

        return shuffled;
    }
}
