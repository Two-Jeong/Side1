using UnityEngine;
using UnityEngine.Pool;

/// <summary>
/// 풀링된 오브젝트에 추가되어, 자신이 속한 풀의 정보를 저장하는 컴포넌트입니다.
/// </summary>
public class PooledObject : MonoBehaviour
{
    public IObjectPool<GameObject> Pool { get; set; }
}