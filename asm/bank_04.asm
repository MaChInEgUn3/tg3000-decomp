; bank $84 (file offset $020000)
org $848000

    incbin !baserom:$20000..$2231B
    ldx.w $0444                 ; $84A31B
    lda.w $03DC                 ; $84A31E
    and.w #$0300                ; $84A321
    db $F0,$0B ; beq            ; $84A324
    cmp.w #$0100                ; $84A326
    beq CODE_84A337             ; $84A329
    incbin !baserom:$2232B..$22337
CODE_84A337:
    lda.l $85D530,x             ; $84A337
    and.w #$00FF                ; $84A33B
    beq CODE_84A34A             ; $84A33E
    incbin !baserom:$22340..$2234A
CODE_84A34A:
    rtl                         ; $84A34A
    ldx.w $0444                 ; $84A34B
    lda.w $03DC                 ; $84A34E
    and.w #$0300                ; $84A351
    db $F0,$0E ; beq            ; $84A354
    cmp.w #$0100                ; $84A356
    beq CODE_84A36D             ; $84A359
    incbin !baserom:$2235B..$2236D
CODE_84A36D:
    lda.l $85D530,x             ; $84A36D
    ldy.w #$001C                ; $84A371
    and.w #$00FF                ; $84A374
    beq CODE_84A387             ; $84A377
    incbin !baserom:$22379..$22387
CODE_84A387:
    rtl                         ; $84A387
    incbin !baserom:$22388..$22487
    lda.l $81047B               ; $84A487
    and.w #$0003                ; $84A48B
    clc                         ; $84A48E
    adc.w #$0003                ; $84A48F
    jsl.l $86DDEF               ; $84A492
    sep.b #$20                  ; $84A496
    lda.b #$7F                  ; $84A498
    sta.w $045D                 ; $84A49A
    sta.w $045E                 ; $84A49D
    rep.b #$20                  ; $84A4A0
    rtl                         ; $84A4A2
    incbin !baserom:$224A3..$224CD
CODE_84A4CD:
    lda.l $8103E0               ; $84A4CD
    bne CODE_84A4CD             ; $84A4D1
    rtl                         ; $84A4D3
    incbin !baserom:$224D4..$224ED
    stz.w $0444                 ; $84A4ED
    ldx.w #$85AB                ; $84A4F0
    ldy.w #$0368                ; $84A4F3
    lda.w #$0015                ; $84A4F6
    mvn $81,$83                 ; $84A4F9
    ldx.w #$85D1                ; $84A4FC
    ldy.w #$038E                ; $84A4FF
    lda.w #$000E                ; $84A502
    mvn $81,$83                 ; $84A505
    ldx.w #$8600                ; $84A508
    ldy.w #$03BD                ; $84A50B
    lda.w #$000E                ; $84A50E
    mvn $81,$83                 ; $84A511
    ldx.w #$0000                ; $84A514
    ldy.w #$000A                ; $84A517
    lda.w #$0100                ; $84A51A
CODE_84A51D:
    sta.w $0446,x               ; $84A51D
    clc                         ; $84A520
    adc.w #$0202                ; $84A521
    inx                         ; $84A524
    inx                         ; $84A525
    dey                         ; $84A526
    bne CODE_84A51D             ; $84A527
    stz.w $036C                 ; $84A529
    lda.w $03E2                 ; $84A52C
    bit.w #$0002                ; $84A52F
    beq CODE_84A552             ; $84A532
    incbin !baserom:$22534..$22552
CODE_84A552:
    rtl                         ; $84A552
    incbin !baserom:$22553..$28000
