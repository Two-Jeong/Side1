using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using DG.Tweening;
using UnityEngine.Events;
using TMPro;
using System;

/// <summary> 버튼 스케일 효과 </summary>
public class UIButton : MonoBehaviour, IPointerDownHandler, IPointerUpHandler, IPointerExitHandler, IPointerClickHandler, IDragHandler
{
    public int ButtonId { get; private set; } = 0;

    [SerializeField] private Transform transformScale;
    [SerializeField] private bool isInteractable = true;
    [SerializeField] private GameObject objRedDot;

    [SerializeField] private GameObject buttonCover;

    //[SerializeField] private LocalizationText localText;

    [Header("Click Event")]
    public UnityEvent onClick;

    [Header("Sound")]
    [Tooltip("체크하시면 사운드가 재생되지 않습니다.")]
    [SerializeField] private bool dontPlaySound = false;
    [Tooltip("빈 값이면 기본 버튼 사운드가 재생됩니다.")]
    [SerializeField] private string soundClipName = string.Empty;

    public bool interactable
    {
        get { return isInteractable; }
        set
        {
            isInteractable = value;

            if (buttonCover)
                buttonCover.SetActive(!value);
        }
    }

    //public string SetTextLabel
    //{
    //    set
    //    {
    //        if (localText != null)
    //            localText.SetText(value);
    //    }
    //}

    public bool SetCover
    {
        set
        {
            if (buttonCover != null)
                buttonCover.SetActive(value);
        }
    }

    public bool GetCover => buttonCover != null ? buttonCover.activeSelf : false;

    public Action OnPointerDownEvent;
    public Action OnPointerUpEvent;


    private TMP_Text textButton_TMP;
    private Text textButton;
    private Vector3 sizePress;
    private Vector3 sizeNormal;

    private Tween tweenPointerDown = null;
    private Tween tweenPointerUp = null;

    private float scaleSize = 0.95f;

    private void Awake()
    {
        sizeNormal = Vector3.one;

        if (transformScale != null)
            sizeNormal = transformScale.transform.localScale;

        sizePress = sizeNormal * scaleSize;

        if (objRedDot)
            objRedDot.SetActive(false);

        textButton_TMP = GetComponentInChildren<TMP_Text>();
        textButton = GetComponentInChildren<Text>();
    }

    public void SetText(string _str)
    {
        if (textButton_TMP)
            textButton_TMP.text = _str;

        if (textButton)
            textButton.text = _str;
    }

    public void SetTransform(Transform _rect)
    {
        transformScale = _rect;
    }

    public void SetRedDot(bool _value)
    {
        if (objRedDot)
            objRedDot.SetActive(_value);
    }

    public void SetId(int _id)
    {
        ButtonId = _id;
    }

    public void Reset()
    {
        if (transformScale == null)
            transformScale = this.transform;
    }

    public virtual void OnPointerDown(PointerEventData eventData)
    {
        if (!interactable)
            return;

        EventSystem.current.SetSelectedGameObject(gameObject);

        PointerDown();

        OnPointerDownEvent?.Invoke();

        PlaySound();
    }

    public virtual void OnPointerUp(PointerEventData eventData)
    {
        //EventSystem.current.SetSelectedGameObject(null);

        PointerUp();

        OnPointerUpEvent?.Invoke();
    }

    public virtual void OnPointerExit(PointerEventData eventData) // 벗어나면 취소
    {
        PointerUp();
    }

    public virtual void OnPointerClick(PointerEventData eventData)
    {
        if (interactable)
        {
            onClick?.Invoke();
            //Vibration.Vibrate(4);
        }

        EventSystem.current.SetSelectedGameObject(null);
        PointerUp();
    }

    private void PointerDown()
    {
        if (transformScale == null)
            return;

        if (tweenPointerDown != null)
        {
            tweenPointerDown.Kill();
            tweenPointerDown = null;
        }

        tweenPointerDown = transformScale.DOScale(sizePress, 0.05f).SetUpdate(true);
    }

    private void PointerUp()
    {
        if (transformScale == null)
            return;

        if (tweenPointerUp != null)
        {
            tweenPointerUp.Kill();
            tweenPointerUp = null;
        }

        if (transformScale.localScale != sizeNormal)
            tweenPointerUp = transformScale.DOScale(sizeNormal, 0.05f).SetUpdate(true);
    }

    private ScrollRect parentScroll = null;
    public void OnDrag(PointerEventData eventData)
    {
        if (parentScroll == null)
            parentScroll = GetComponentInParent<ScrollRect>();

        if (parentScroll != null)
        {
            eventData.pointerDrag = parentScroll.gameObject;
            EventSystem.current.SetSelectedGameObject(parentScroll.gameObject);

            parentScroll.OnInitializePotentialDrag(eventData);
            parentScroll.OnBeginDrag(eventData);
        }
    }

    private void PlaySound()
    {
        if (dontPlaySound)
            return;

        if (string.IsNullOrEmpty(soundClipName))
            SoundManager.Instance.PlayButton();
        else
            SoundManager.Instance.PlayEffect(soundClipName);
    }
}
