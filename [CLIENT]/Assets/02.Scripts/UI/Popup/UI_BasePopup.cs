using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public abstract class UI_BasePopup : MonoBehaviour
{
    /// <summary> 뒤로가기 버튼을 막을 건지 </summary>
    public virtual bool IsDontCloseBackButton { get; internal set; } = false;
    /// <summary> 필드 터치를 막을 건지 </summary>
    public abstract bool IsDontTouchField { get; }
    /// <summary> 팝업을 열 때 다른 팝업을 닫을 건지 </summary>
    public virtual bool IsCloseOtherPopups { get; internal set; } = false;
    /// <summary> 팝업을 열 때 메인 UI를 안보이게 할 건지 </summary>
    public virtual bool IsCloseMainUI { get; internal set; } = true;
    /// <summary> 뒤로가기 버튼 기능을 팝업 내에서 처리할 지 </summary>
    public virtual bool IsCheckClose { get; internal set; } = false;

    [Header("BasePopup")]
    [Tooltip("체크하시면 사운드가 재생되지 않습니다.")]
    [SerializeField] private bool dontPlaySound = false;
    [Tooltip("빈 값이면 기본 사운드가 재생됩니다.")]
    [SerializeField] private string soundClipName = string.Empty;

    public bool IsOpen => gameObject.activeSelf;

    private void Awake()
    {
        //if (buttonLevelUp)
        //    buttonLevelUp.onClick.AddListener(OnClickLevelUp);

        OnAwake();
    }

    protected virtual void OnAwake()
    {

    }

    public void Open()
    {
        PlaySoundPopupOn();
        UIManager.Instance.AddOpenPopup(this);
        gameObject.SetActive(true);
    }

    public virtual void CheckClose()
    {
        Close();
    }

    public virtual void Close()
    {
        PlaySoundPopupOff();
        gameObject.SetActive(false);

        UIManager.Instance.RemoveOpenPopup(this);
    }

    private void PlaySoundPopupOn()
    {
        if (dontPlaySound)
            return;

        if (string.IsNullOrEmpty(soundClipName))
            SoundManager.Instance.PlayEffect("sfx_common_popup_open");
        else
            SoundManager.Instance.PlayEffect(soundClipName);
    }

    private void PlaySoundPopupOff()
    {
        if (dontPlaySound)
            return;

        //if(string.IsNullOrEmpty(soundClipName))
        //    SoundManager.Instance.PlayEffect("Popup_Off");
        //else
        //    SoundManager.Instance.PlayEffect(soundClipName);
    }

}
