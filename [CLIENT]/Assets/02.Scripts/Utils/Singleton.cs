using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class Singleton<T> : MonoBehaviour where T : MonoBehaviour
{
    private static T _instance;

    public static T Instance
    {
        get
        {
            if (_instance == null)
                _instance = FindObjectOfType<T>();

            if (_instance == null)
            {
                var obj = new GameObject(typeof(T).ToString());
                _instance = obj.AddComponent<T>();
            }

            return _instance;
        }
    }

    protected virtual bool DontDestroyOnload => false;

    protected void Awake()
    {
        if (CheckAnotherInstance())
        {
            return;
        }

        T _ = Instance;
        if (DontDestroyOnload)
        {
            DontDestroyOnLoad(gameObject);
        }

        OnAwake();
    }

    protected virtual void OnAwake() { }

    private bool CheckAnotherInstance()
    {
        T[] objects = FindObjectsOfType<T>();
        int count = objects.Length;
        if (count >= 2)
        {
            foreach (T a in objects)
            {
                if (a != _instance)
                {
                    DebugLog.Log($"Duplicated singleton object [{a.gameObject.name}] destoryed");
                    Destroy(a.gameObject);
                }
            }

            return true;
        }

        return false;
    }

    protected virtual void OnDestroy()
    {
        if (_instance == this)
        {
            _instance = null;
        }
    }
}
