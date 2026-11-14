; bank $8E (file offset $070000)
org $8E8000

    incbin !baserom:$70000..$74823
    ora.b ($C5,x)               ; $8EC823
    eor.w #$1101                ; $8EC825
    cmp.b $AD                   ; $8EC828
    cpy.w #$064D                ; $8EC82A
    rol a                       ; $8EC82D
    jsr.w $FFE8                 ; $8EC82E
    incbin !baserom:$74831..$77FE8
    ora.w $43D4                 ; $8EFFE8
    ora.b ($00),y               ; $8EFFEB
    inc.w $DFC6,x               ; $8EFFED
    ora.b ($01),y               ; $8EFFF0
    brk.b #$00                  ; $8EFFF2
    incbin !baserom:$77FF4..$78000
