using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public static class EventManager
{
    public delegate void EventDelegate(params object[] parameters);

    private static Dictionary<EventType, EventDelegate> _dicEvents;
    private static Dictionary<EventType, EventDelegate> dicEvents
    {
        get
        {
            if (null == _dicEvents)
                Release();
            return _dicEvents;
        }
        set
        {
            _dicEvents = value;
        }
    }

    // 초기화
    public static void Release()
    {
        dicEvents = new Dictionary<EventType, EventDelegate>();
    }

    // 등록
    public static void Add_Event(EventType type, EventDelegate callBack)
    {
        if (dicEvents.ContainsKey(type))
        {
            if (dicEvents[type].Equals(callBack))
                DebugLog.LogError($"[이벤트 중복 딜리게이트 등록시도] {type}");
            else
                dicEvents[type] += callBack;
        }
        else
            dicEvents.Add(type, callBack);
    }

    // 제거
    public static void Remove_Event(EventType type, EventDelegate callBack)
    {
        if (dicEvents.ContainsKey(type))
        {
            if (dicEvents[type].GetInvocationList().Length == 1 && dicEvents[type].Equals(callBack))
                dicEvents.Remove(type);
            else
                dicEvents[type] -= callBack;
        }
    }

    // 호출
    public static void Invoke_Event(EventType type, params object[] value)
    {
        if (dicEvents.ContainsKey(type))
        {
            dicEvents[type].Invoke(value);
        }
    }
}