; bank $84 (file offset $020000)
org $848000

    incbin !baserom:$20000..$2231B
result_slot_step:
    ldx.w $0444                 ; $84A31B
    lda.w $03DC                 ; $84A31E
    and.w #$0300                ; $84A321
    beq CODE_84A331             ; $84A324
    cmp.w #$0100                ; $84A326
    beq CODE_84A337             ; $84A329
    lda.l $85D55F,x             ; $84A32B
    bra CODE_84A33B             ; $84A32F
CODE_84A331:
    lda.l $85D501,x             ; $84A331 static
    bra CODE_84A33B             ; $84A335 static
CODE_84A337:
    lda.l $85D530,x             ; $84A337
CODE_84A33B:
    and.w #$00FF                ; $84A33B
    beq CODE_84A34A             ; $84A33E
    lda.w $036C                 ; $84A340
    clc                         ; $84A343
    adc.w #$0004                ; $84A344
    sta.w $036C                 ; $84A347
CODE_84A34A:
    rtl                         ; $84A34A
result_tally:
    ldx.w $0444                 ; $84A34B
    lda.w $03DC                 ; $84A34E
    and.w #$0300                ; $84A351
    beq CODE_84A364             ; $84A354
    cmp.w #$0100                ; $84A356
    beq CODE_84A36D             ; $84A359
    lda.l $85D55F,x             ; $84A35B
    ldy.w #$0044                ; $84A35F
    bra CODE_84A374             ; $84A362
CODE_84A364:
    lda.l $85D501,x             ; $84A364 static
    ldy.w #$0000                ; $84A368 static
    bra CODE_84A374             ; $84A36B static
CODE_84A36D:
    lda.l $85D530,x             ; $84A36D
    ldy.w #$001C                ; $84A371
CODE_84A374:
    and.w #$00FF                ; $84A374
    beq CODE_84A387             ; $84A377
    tya                         ; $84A379
    clc                         ; $84A37A
    adc.w $036C                 ; $84A37B
    tax                         ; $84A37E
    lda.l $85D590,x             ; $84A37F
    tax                         ; $84A383
    inc.w $036E,x               ; $84A384
CODE_84A387:
    rtl                         ; $84A387
prize_money_award:
    sed                         ; $84A388
    lda.w $03A8                 ; $84A389
    bmi CODE_84A3A6             ; $84A38C
    asl a                       ; $84A38E
    asl a                       ; $84A38F
    tax                         ; $84A390
    lda.l $84A2B3,x             ; $84A391
    clc                         ; $84A395
    adc.w $038E                 ; $84A396
    sta.w $038E                 ; $84A399
    lda.l $84A2B5,x             ; $84A39C
    adc.w $0390                 ; $84A3A0
    sta.w $0390                 ; $84A3A3
CODE_84A3A6:
    lda.w $03A8                 ; $84A3A6
    bmi CODE_84A3D1             ; $84A3A9
    ldx.w #$0000                ; $84A3AB
CODE_84A3AE:
    lsr.w $03AB                 ; $84A3AE
    bcc CODE_84A3C8             ; $84A3B1
    lda.l $84A2DB,x             ; $84A3B3
    clc                         ; $84A3B7
    adc.w $038E                 ; $84A3B8
    sta.w $038E                 ; $84A3BB
    lda.l $84A2DD,x             ; $84A3BE
    adc.w $0390                 ; $84A3C2
    sta.w $0390                 ; $84A3C5
CODE_84A3C8:
    inx                         ; $84A3C8
    inx                         ; $84A3C9
    inx                         ; $84A3CA
    inx                         ; $84A3CB
    lda.w $03AB                 ; $84A3CC
    bne CODE_84A3AE             ; $84A3CF
CODE_84A3D1:
    lda.w $03D7                 ; $84A3D1
    bmi CODE_84A3EE             ; $84A3D4
    asl a                       ; $84A3D6
    asl a                       ; $84A3D7
    tax                         ; $84A3D8
    lda.l $84A2B3,x             ; $84A3D9
    clc                         ; $84A3DD
    adc.w $03BD                 ; $84A3DE
    sta.w $03BD                 ; $84A3E1
    lda.l $84A2B5,x             ; $84A3E4
    adc.w $03BF                 ; $84A3E8
    sta.w $03BF                 ; $84A3EB
CODE_84A3EE:
    lda.w $03D7                 ; $84A3EE
    bmi CODE_84A419             ; $84A3F1
    ldx.w #$0000                ; $84A3F3
CODE_84A3F6:
    lsr.w $03DA                 ; $84A3F6
    bcc CODE_84A410             ; $84A3F9
    lda.l $84A2DB,x             ; $84A3FB
    clc                         ; $84A3FF
    adc.w $03BD                 ; $84A400
    sta.w $03BD                 ; $84A403
    lda.l $84A2DD,x             ; $84A406
    adc.w $03BF                 ; $84A40A
    sta.w $03BF                 ; $84A40D
CODE_84A410:
    inx                         ; $84A410
    inx                         ; $84A411
    inx                         ; $84A412
    inx                         ; $84A413
    lda.w $03DA                 ; $84A414
    bne CODE_84A3F6             ; $84A417
CODE_84A419:
    cld                         ; $84A419
    rtl                         ; $84A41A
track_next:
    lda.w $03DC                 ; $84A41B
    and.w #$0300                ; $84A41E
    beq CODE_84A42D             ; $84A421
    cmp.w #$0100                ; $84A423
    beq CODE_84A44B             ; $84A426
    cmp.w #$0200                ; $84A428
    beq CODE_84A469             ; $84A42B
CODE_84A42D:
    ldx.w #$0000                ; $84A42D static
CODE_84A430:
    lda.l $8ACAC4,x             ; $84A430 static
    and.w #$00FF                ; $84A434 static
    cmp.w $0444                 ; $84A437 static
    beq CODE_84A43F             ; $84A43A static
    inx                         ; $84A43C static
    bra CODE_84A430             ; $84A43D static
CODE_84A43F:
    inx                         ; $84A43F static
    lda.l $8ACAC4,x             ; $84A440 static
    and.w #$00FF                ; $84A444 static
    sta.w $0444                 ; $84A447 static
    rtl                         ; $84A44A static
CODE_84A44B:
    ldx.w #$0000                ; $84A44B
CODE_84A44E:
    lda.l $8ACADE,x             ; $84A44E
    and.w #$00FF                ; $84A452
    cmp.w $0444                 ; $84A455
    beq CODE_84A45D             ; $84A458
    inx                         ; $84A45A
    bra CODE_84A44E             ; $84A45B
CODE_84A45D:
    inx                         ; $84A45D
    lda.l $8ACADE,x             ; $84A45E
    and.w #$00FF                ; $84A462
    sta.w $0444                 ; $84A465
    rtl                         ; $84A468
CODE_84A469:
    ldx.w #$0000                ; $84A469
CODE_84A46C:
    lda.l $8ACB01,x             ; $84A46C
    and.w #$00FF                ; $84A470
    cmp.w $0444                 ; $84A473
    beq CODE_84A47B             ; $84A476
    inx                         ; $84A478
    bra CODE_84A46C             ; $84A479
CODE_84A47B:
    inx                         ; $84A47B
    lda.l $8ACB01,x             ; $84A47C
    and.w #$00FF                ; $84A480
    sta.w $0444                 ; $84A483
    rtl                         ; $84A486
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
    incbin !baserom:$224A3..$224B7
    jsl.l $86DDEF               ; $84A4B7
    sep.b #$20                  ; $84A4BB
    lda.b #$01                  ; $84A4BD
    sta.w $045F                 ; $84A4BF
    lda.b #$7F                  ; $84A4C2
    sta.w $045D                 ; $84A4C4
    sta.w $045E                 ; $84A4C7
    rep.b #$20                  ; $84A4CA
    rtl                         ; $84A4CC
wait_03e0:
    lda.l $8103E0               ; $84A4CD
    bne wait_03e0               ; $84A4D1
    rtl                         ; $84A4D3
    incbin !baserom:$224D4..$224ED
results_init:
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
    sep.b #$20                  ; $84A534 static
    lda.b #$01                  ; $84A536 static
    sta.w $0398                 ; $84A538 static
    sta.w $0399                 ; $84A53B static
    sta.w $039A                 ; $84A53E static
    sta.w $039C                 ; $84A541 static
    sta.w $03C7                 ; $84A544 static
    sta.w $03C8                 ; $84A547 static
    sta.w $03C9                 ; $84A54A static
    sta.w $03CB                 ; $84A54D static
    rep.b #$20                  ; $84A550 static
CODE_84A552:
    rtl                         ; $84A552
    incbin !baserom:$22553..$28000
