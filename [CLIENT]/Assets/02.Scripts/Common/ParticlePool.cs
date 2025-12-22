using System.Collections;
using UnityEngine;

public class ParticlePool : ObjectPool
{
    private ParticleSystem particle;

    private void Awake()
    {
        particle = GetComponentInChildren<ParticleSystem>();
    }

    private void OnEnable()
    {
        if (particle != null)
            StartCoroutine("DoUpdate");
    }

    private IEnumerator DoUpdate()
    {
        do
        {
            yield return null;
        }
        while (particle.IsAlive(true));

        Release();
    }
}