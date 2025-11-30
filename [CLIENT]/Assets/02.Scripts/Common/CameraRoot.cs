using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraRoot : Singleton<CameraRoot>
{
    protected override bool DontDestroyOnload => true;

    [SerializeField] private Camera uiCamera;

    public Camera UICamera
    {
        get { return uiCamera; }
    }

    [SerializeField] private Camera mainCamera;

    public Camera MainCamera
    {
        get { return mainCamera; }
    }
}
