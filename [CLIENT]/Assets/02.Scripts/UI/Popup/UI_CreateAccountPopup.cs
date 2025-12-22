using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class UI_CreateAccountPopup : UI_BasePopup
{
    public override bool IsDontTouchField => true;

    [SerializeField] private UIButton btnClose;
    [SerializeField] private UIButton btnCreate;

    public void Awake()
    {
        btnClose.onClick.AddListener(OnBtnCloseClick);
        btnCreate.onClick.AddListener(OnBtnCreateClick);
    }

    private void OnBtnCloseClick()
    {
        Close();
    }

    private void OnBtnCreateClick()
    {
        
    }

    public void Show()
    {

    }
}
