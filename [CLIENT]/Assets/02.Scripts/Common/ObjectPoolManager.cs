using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Pool;
using System.Linq;

public class ObjectPoolManager : SingletonNoCreate<ObjectPoolManager>
{
    protected override bool DontDestroyOnload => false;

    [System.Serializable]
    public class GameObjectPoolItemInfo
    {
        [Tooltip("풀의 이름으로 사용될 프리팹의 이름입니다.")]
        public string Name;
        [Tooltip("풀의 초기 생성 개수입니다.")]
        public int InitialCount = 10;
        [Tooltip("풀링할 게임 오브젝트 프리팹입니다.")]
        public GameObject Prefab;
    }

    [SerializeField] private List<GameObjectPoolItemInfo> poolItems;

    private readonly Dictionary<string, IObjectPool<GameObject>> _pools = new Dictionary<string, IObjectPool<GameObject>>();
    private int maxPoolSize = 100;

    protected override void OnAwake()
    {
        Init();
    }

    private void Init()
    {
        foreach (GameObjectPoolItemInfo poolInfo in poolItems)
        {
            // 프리팹이 할당되지 않았거나 이름이 없으면 건너뜁니다.
            if (poolInfo.Prefab == null)
            {
                DebugLog.LogWarning("PoolItems 리스트에 프리팹이 할당되지 않은 항목이 있습니다.");
                continue;
            }
            if (string.IsNullOrEmpty(poolInfo.Name))
            {
                poolInfo.Name = poolInfo.Prefab.name;
            }
            CreatePoolObjects(poolInfo);
        }
    }

    public void ReleaseObject(GameObject _gameObject)
    {
        // PooledObject 컴포넌트를 통해 자신을 생성한 풀을 찾아 반환합니다.
        if (_gameObject.TryGetComponent<PooledObject>(out var pooledObject) && pooledObject.Pool != null)
        {
            pooledObject.Pool.Release(_gameObject);
        }
        else
        {
            DebugLog.LogWarning($"'{_gameObject.name}' 오브젝트는 풀링된 객체가 아니거나, 풀 정보를 잃어버렸습니다. 즉시 파괴합니다.");
            Destroy(_gameObject);
        }
    }

    /// <summary> 오브젝트 가져오기 </summary>
    public GameObject GetObject(string _name)
    {
        if (_pools.TryGetValue(_name, out var pool))
        {
            return pool.Get();
        }

        DebugLog.LogError($"'${_name}' 이름의 오브젝트 풀이 존재하지 않습니다.");
        return null;
    }

    /// <summary> 런타임에 풀을 생성하고 오브젝트를 가져옵니다. </summary>
    public GameObject GetObjectOrCreate(GameObject prefab, int initialCount = 5)
    {
        if (prefab == null)
            return null;

        if (!_pools.ContainsKey(prefab.name))
        {
            DebugLog.Log($"'{prefab.name}' 풀을 런타임에 새로 생성합니다.");
            var info = new GameObjectPoolItemInfo
            {
                Name = prefab.name,
                InitialCount = initialCount,
                Prefab = prefab
            };
            CreatePoolObjects(info);
        }

        return GetObject(prefab.name);
    }

    // /// <summary>
    // /// 특정 풀의 활성화된 오브젝트 개수를 반환합니다.
    // /// </summary>
    // public int GetActiveObjectCount(string poolName)
    // {
    //     if (_pools.TryGetValue(poolName, out var pool))
    //     {
    //         return pool.CountInactive;
    //     }
    //     return 0;
    // }

    #region Create
    private void CreatePoolObjects(GameObjectPoolItemInfo _poolInfo)
    {
        if (_poolInfo == null || _poolInfo.Prefab == null)
            return;

        if (_pools.ContainsKey(_poolInfo.Name) == false)
        {
            var newPool = new ObjectPool<GameObject>(
                () => { return CreatePooledItem(_poolInfo); },
                OnTakeFromPool,
                OnReturnedToPool,
                OnDestroyPoolObject,
                true, _poolInfo.InitialCount, maxPoolSize);

            _pools.Add(_poolInfo.Name, newPool);

            // [수정] 오브젝트를 미리 생성하여 풀을 준비시킵니다 (Pre-warming).
            var prewarmedObjects = new List<GameObject>();
            for (int i = 0; i < _poolInfo.InitialCount; i++)
            {
                prewarmedObjects.Add(newPool.Get());
            }
            foreach (var obj in prewarmedObjects)
            {
                newPool.Release(obj);
            }
        }
    }

    // 생성
    private GameObject CreatePooledItem(GameObjectPoolItemInfo _poolInfo)
    {
        GameObject poolGo = Instantiate(_poolInfo.Prefab);
        poolGo.name = _poolInfo.Name; // 이름에서 '(Clone)' 접미사 제거

        // [수정] PooledObject 컴포넌트를 추가하고 풀 정보를 저장합니다.
        var pooledObject = poolGo.AddComponent<PooledObject>();
        pooledObject.Pool = _pools[_poolInfo.Name];

        return poolGo;
    }

    // 사용
    private void OnTakeFromPool(GameObject poolGo)
    {
        poolGo.SetActive(true);
    }

    // 반환
    private void OnReturnedToPool(GameObject poolGo)
    {
        poolGo.SetActive(false);
    }

    // 삭제
    private void OnDestroyPoolObject(GameObject poolGo)
    {
        Destroy(poolGo);
    }
    #endregion

    [ContextMenu("SetName")]
    private void SetName()
    {
        foreach (GameObjectPoolItemInfo info in poolItems)
        {
            if (info.Prefab != null)
            {
                info.Name = info.Prefab.name;
            }
        }
#if UNITY_EDITOR
        UnityEditor.EditorUtility.SetDirty(this);
#endif
    }
}
