; bank $8A (file offset $050000)
org $8A8000

func_8A8000:
    rtl                         ; $8A8000
clear_dp_scratch:
    ldx.w #$007E                ; $8A8001
CODE_8A8004:
    stz.b $14,x                 ; $8A8004
    dex                         ; $8A8006
    dex                         ; $8A8007
    bpl CODE_8A8004             ; $8A8008
    rtl                         ; $8A800A
title_screen:
    stz.w $0196                 ; $8A800B
    stz.w $0198                 ; $8A800E
    stz.w $019A                 ; $8A8011
    sep.b #$20                  ; $8A8014
    stz.w $420C                 ; $8A8016
    stz.w $420B                 ; $8A8019
    rep.b #$20                  ; $8A801C
    ldx.w #$9307                ; $8A801E
    lda.w #$008A                ; $8A8021
    jsl.l video_load_screen_long; $8A8024
    lda.w #$FBD1                ; $8A8028
    sta.w $0008                 ; $8A802B
    jsl.l clear_dp_scratch      ; $8A802E
    jsl.l clear_vram            ; $8A8032
    stz.b $8C                   ; $8A8036
    ldx.w #$001E                ; $8A8038
CODE_8A803B:
    stz.w $1516,x               ; $8A803B
    dex                         ; $8A803E
    dex                         ; $8A803F
    bpl CODE_8A803B             ; $8A8040
title_screen_scene:
    lda.w #$0001                ; $8A8042
    jsl.l wait_frames_8a8bb6    ; $8A8045
    jsr.w track_preview_maybe   ; $8A8049
    jsl.l hdma_gradients_init   ; $8A804C
    jsl.l clear_dp_scratch      ; $8A8050
    jsl.l func_8A8EEF           ; $8A8054
    jsr.w vram_planes_build     ; $8A8058
    jsr.w title_scene_load      ; $8A805B
    jsr.w mode7_view            ; $8A805E
    jsl.l oam_from_objects      ; $8A8061
    jsr.w hdma_title_setup      ; $8A8065
    lda.w #$0087                ; $8A8068
    sta.b $46                   ; $8A806B
    lda.w #$82CA                ; $8A806D
    sta.b $44                   ; $8A8070
    lda.w #$0008                ; $8A8072
    sta.b $56                   ; $8A8075
    lda.w #$FFFF                ; $8A8077
    sta.b $62                   ; $8A807A
    lda.w #$0001                ; $8A807C
    sta.w $050A                 ; $8A807F
    sta.w $050C                 ; $8A8082
    jsr.w scene_step            ; $8A8085
    jsr.w func_8A8232           ; $8A8088
    jsr.w func_8A823D           ; $8A808B
    jsr.w func_8A8290           ; $8A808E
    jsr.w title_sprites_place   ; $8A8091
    jsr.w rotation_from_angle   ; $8A8094
    jsr.w func_8A869A           ; $8A8097
    jsr.w scene_objects_place   ; $8A809A
    jsr.w screen_objects_place  ; $8A809D
    jsl.l oam_from_objects      ; $8A80A0
    lda.w #$0004                ; $8A80A4
    sta.b $8A                   ; $8A80A7
    sep.b #$20                  ; $8A80A9
    lda.b #$21                  ; $8A80AB
    sta.w $4361                 ; $8A80AD
    lda.b #$43                  ; $8A80B0
    sta.w $4360                 ; $8A80B2
    ldx.w #$2C00                ; $8A80B5
    stx.w $4362                 ; $8A80B8
    lda.b #$7E                  ; $8A80BB
    sta.w $4364                 ; $8A80BD
    sta.w $4367                 ; $8A80C0
    rep.b #$20                  ; $8A80C3
    jsr.w palettes_load_8a      ; $8A80C5
title_screen_show:
    lda.w #$0001                ; $8A80C8
    jsl.l wait_frames_8a8bb6    ; $8A80CB
    sep.b #$20                  ; $8A80CF
    lda.b #$FE                  ; $8A80D1
    sta.w $420C                 ; $8A80D3
    lda.b #$81                  ; $8A80D6
    sta.w $4200                 ; $8A80D8
    rep.b #$20                  ; $8A80DB
    lda.w #$0F00                ; $8A80DD
    sta.b $98                   ; $8A80E0
    lda.w #$0080                ; $8A80E2
    sta.b $96                   ; $8A80E5
    jsr.w fade_out_on_confirm_once; $8A80E7
    lda.w #$0001                ; $8A80EA
    jsl.l wait_frames_8a8bb6    ; $8A80ED
    jsr.w mode7_view            ; $8A80F1
    lda.b $5A                   ; $8A80F4
    beq CODE_8A8142             ; $8A80F6
    incbin !baserom:$500F8..$50142
CODE_8A8142:
    jsr.w tile_anim_step        ; $8A8142
    lda.w #$0002                ; $8A8145
    tsb.w $0026                 ; $8A8148
    jsr.w scene_step            ; $8A814B
    jsr.w func_8A8232           ; $8A814E
    jsr.w func_8A823D           ; $8A8151
    jsr.w func_8A8290           ; $8A8154
    jsr.w title_sprites_place   ; $8A8157
    jsr.w rotation_from_angle   ; $8A815A
    jsr.w scene_objects_place   ; $8A815D
    jsr.w screen_objects_place  ; $8A8160
    jsr.w func_8A869A           ; $8A8163
    jsl.l oam_from_objects      ; $8A8166
    lda.b $58                   ; $8A816A
    bne CODE_8A8179             ; $8A816C
    lda.b $8C                   ; $8A816E
    beq CODE_8A8176             ; $8A8170
    lda.b $9A                   ; $8A8172
    beq CODE_8A8179             ; $8A8174
CODE_8A8176:
    jmp.w $80E7                 ; $8A8176
CODE_8A8179:
    lda.w #$0008                ; $8A8179
    jsl.l wait_frames_8a8bb6    ; $8A817C
    rtl                         ; $8A8180
fade_out_on_confirm_once:
    lda.b $8C                   ; $8A8181
    bne CODE_8A81B9             ; $8A8183
    lda.w #$C0C0                ; $8A8185
    trb.w $000E                 ; $8A8188
    bne CODE_8A81B0             ; $8A818B
    lda.w #$C0C0                ; $8A818D
    trb.w $0012                 ; $8A8190
    bne CODE_8A81B0             ; $8A8193
    rts                         ; $8A8195
    incbin !baserom:$50196..$501B0
CODE_8A81B0:
    stz.b $98                   ; $8A81B0
    lda.w #$FF80                ; $8A81B2
    sta.b $96                   ; $8A81B5
    inc.b $8C                   ; $8A81B7
CODE_8A81B9:
    rts                         ; $8A81B9
palettes_load_8a:
    lda.w #$7E80                ; $8A81BA
    ldx.w #$81E2                ; $8A81BD
    ldy.w #$0020                ; $8A81C0
    jsl.l upload_cgram_now      ; $8A81C3
    lda.w #$7E90                ; $8A81C7
    ldx.w #$8202                ; $8A81CA
    ldy.w #$0020                ; $8A81CD
    jsl.l upload_cgram_now      ; $8A81D0
    lda.w #$7EA0                ; $8A81D4
    ldx.w #$8222                ; $8A81D7
    ldy.w #$0020                ; $8A81DA
    jsl.l upload_cgram_now      ; $8A81DD
    lda.w #$7E01                ; $8A81E1
    ldx.w #$8114                ; $8A81E4
    ldy.w #$001E                ; $8A81E7
    jsl.l upload_cgram_now      ; $8A81EA
    lda.w $1536                 ; $8A81EE
    asl a                       ; $8A81F1
    asl a                       ; $8A81F2
    asl a                       ; $8A81F3
    tax                         ; $8A81F4
    lda.l $8A934F,x             ; $8A81F5
    tax                         ; $8A81F9
    lda.w #$7EA0                ; $8A81FA
    ldy.w #$0020                ; $8A81FD
    jsl.l upload_cgram_now      ; $8A8200
    rts                         ; $8A8204
    incbin !baserom:$50205..$50232
func_8A8232:
    lda.b $36                   ; $8A8232
    lsr a                       ; $8A8234
    lsr a                       ; $8A8235
    clc                         ; $8A8236
    adc.w #$0108                ; $8A8237
    sta.b $4A                   ; $8A823A
    rts                         ; $8A823C
func_8A823D:
    lda.b $38                   ; $8A823D
    lsr a                       ; $8A823F
    lsr a                       ; $8A8240
    clc                         ; $8A8241
    adc.w #$FFB7                ; $8A8242
    clc                         ; $8A8245
    adc.b $64                   ; $8A8246
    sta.b $4C                   ; $8A8248
    rts                         ; $8A824A
title_sprites_place:
    lda.b $62                   ; $8A824B
    cmp.w #$FFFF                ; $8A824D
    beq CODE_8A8253             ; $8A8250
    rts                         ; $8A8252 static
CODE_8A8253:
    lda.w #$00A4                ; $8A8253
    sta.w $080A                 ; $8A8256
    lda.b $4A                   ; $8A8259
    eor.w #$FFFF                ; $8A825B
    inc a                       ; $8A825E
    clc                         ; $8A825F
    adc.w #$404D                ; $8A8260
    clc                         ; $8A8263
    adc.b $74                   ; $8A8264
    sta.w $060A                 ; $8A8266
    lda.b $4C                   ; $8A8269
    eor.w #$FFFF                ; $8A826B
    inc a                       ; $8A826E
    clc                         ; $8A826F
    adc.w #$0053                ; $8A8270
    clc                         ; $8A8273
    adc.b $76                   ; $8A8274
    sta.w $070A                 ; $8A8276
    lda.w #$00A6                ; $8A8279
    sta.w $080C                 ; $8A827C
    lda.w $060A                 ; $8A827F
    clc                         ; $8A8282
    adc.w #$0010                ; $8A8283
    sta.w $060C                 ; $8A8286
    lda.w $070A                 ; $8A8289
    sta.w $070C                 ; $8A828C
    rts                         ; $8A828F
func_8A8290:
    lda.b $54                   ; $8A8290
    bne CODE_8A8295             ; $8A8292
    rts                         ; $8A8294
CODE_8A8295:
    inc.b $52                   ; $8A8295 static
    lda.b $52                   ; $8A8297 static
    and.w #$0007                ; $8A8299 static
    beq CODE_8A829F             ; $8A829C static
    rts                         ; $8A829E static
CODE_8A829F:
    ldx.b $50                   ; $8A829F static
    lda.l $8A82B4,x             ; $8A82A1 static
    sta.b $64                   ; $8A82A5 static
    inx                         ; $8A82A7 static
    inx                         ; $8A82A8 static
    cpx.w #$000C                ; $8A82A9 static
    bne CODE_8A82B1             ; $8A82AC static
    ldx.w #$0000                ; $8A82AE static
CODE_8A82B1:
    stx.b $50                   ; $8A82B1 static
    rts                         ; $8A82B3 static
    incbin !baserom:$502B4..$502C0
scene_step:
    lda.b $48                   ; $8A82C0
    beq CODE_8A82C7             ; $8A82C2
    dec.b $48                   ; $8A82C4
    rts                         ; $8A82C6
CODE_8A82C7:
    jmp.w ($0144)               ; $8A82C7
    stz.b $54                   ; $8A82CA
    lda.b $3C                   ; $8A82CC
    clc                         ; $8A82CE
    adc.b $56                   ; $8A82CF
    sta.b $3C                   ; $8A82D1
    lda.b $36                   ; $8A82D3
    sec                         ; $8A82D5
    sbc.b $56                   ; $8A82D6
    sta.b $36                   ; $8A82D8
    dec.b $46                   ; $8A82DA
    bne CODE_8A82E8             ; $8A82DC
    lda.w #$0040                ; $8A82DE
    sta.b $46                   ; $8A82E1
    lda.w #$82EC                ; $8A82E3
    sta.b $44                   ; $8A82E6
CODE_8A82E8:
    jsr.w func_8A8325           ; $8A82E8
    rts                         ; $8A82EB
    stz.b $54                   ; $8A82EC
    lda.b $3C                   ; $8A82EE
    clc                         ; $8A82F0
    adc.b $56                   ; $8A82F1
    sta.b $3C                   ; $8A82F3
    lda.b $36                   ; $8A82F5
    sec                         ; $8A82F7
    sbc.b $56                   ; $8A82F8
    sta.b $36                   ; $8A82FA
    inc.b $60                   ; $8A82FC
    lda.b $60                   ; $8A82FE
    and.w #$0001                ; $8A8300
    bne CODE_8A830B             ; $8A8303
    lda.b $56                   ; $8A8305
    beq CODE_8A830B             ; $8A8307
    dec.b $56                   ; $8A8309
CODE_8A830B:
    jsr.w func_8A8325           ; $8A830B
    dec.b $46                   ; $8A830E
    bne CODE_8A8321             ; $8A8310
    lda.w #$0008                ; $8A8312
    sta.b $48                   ; $8A8315
    lda.w #$0036                ; $8A8317
    sta.b $46                   ; $8A831A
    lda.w #$833D                ; $8A831C
    sta.b $44                   ; $8A831F
CODE_8A8321:
    rts                         ; $8A8321
    inc.b $58                   ; $8A8322 static
    rts                         ; $8A8324 static
func_8A8325:
    lda.b $3C                   ; $8A8325
    lsr a                       ; $8A8327
    lsr a                       ; $8A8328
    sec                         ; $8A8329
    sbc.w #$00CA                ; $8A832A
    sta.w $0600                 ; $8A832D
    lda.b $3E                   ; $8A8330
    lsr a                       ; $8A8332
    lsr a                       ; $8A8333
    clc                         ; $8A8334
    adc.w #$001B                ; $8A8335
    sta.w $0700                 ; $8A8338
    rts                         ; $8A833B
    rts                         ; $8A833C static
    inc.w $0600                 ; $8A833D static
    lda.w $0600                 ; $8A8340 static
    cmp.w #$006C                ; $8A8343 static
    bne CODE_8A834D             ; $8A8346 static
    lda.w #$0001                ; $8A8348 static
    sta.b $42                   ; $8A834B static
CODE_8A834D:
    dec.b $46                   ; $8A834D static
    bne CODE_8A8360             ; $8A834F static
    lda.w #$0008                ; $8A8351 static
    sta.b $48                   ; $8A8354 static
    lda.w #$0000                ; $8A8356 static
    sta.b $46                   ; $8A8359 static
    lda.w #$8361                ; $8A835B static
    sta.b $44                   ; $8A835E static
CODE_8A8360:
    rts                         ; $8A8360 static
    lda.w #$0006                ; $8A8361 static
    sta.b $48                   ; $8A8364 static
    ldx.b $46                   ; $8A8366 static
    lda.l $7E8096,x             ; $8A8368 static
    sta.w $0800                 ; $8A836C static
    inx                         ; $8A836F static
    inx                         ; $8A8370 static
    cpx.w #$0024                ; $8A8371 static
    bne CODE_8A8390             ; $8A8374 static
    stz.w $0500                 ; $8A8376 static
    lda.w #$0020                ; $8A8379 static
    sta.b $48                   ; $8A837C static
    lda.w #$8393                ; $8A837E static
    sta.b $44                   ; $8A8381 static
    lda.w #$000E                ; $8A8383 static
    sta.b $46                   ; $8A8386 static
    lda.w #$0001                ; $8A8388 static
    sta.b $5A                   ; $8A838B static
    inc.b $5C                   ; $8A838D static
    rts                         ; $8A838F static
CODE_8A8390:
    stx.b $46                   ; $8A8390 static
    rts                         ; $8A8392 static
    inc.b $4E                   ; $8A8393 static
    lda.b $4E                   ; $8A8395 static
    and.w #$0007                ; $8A8397 static
    beq CODE_8A839D             ; $8A839A static
    rts                         ; $8A839C static
CODE_8A839D:
    inc.b $5C                   ; $8A839D static
    dec.b $46                   ; $8A839F static
    bne CODE_8A83B9             ; $8A83A1 static
    lda.w #$83D6                ; $8A83A3 static
    sta.b $44                   ; $8A83A6 static
    lda.w #$005A                ; $8A83A8 static
    sta.b $46                   ; $8A83AB static
    stz.b $56                   ; $8A83AD static
    lda.w #$0002                ; $8A83AF static
    sta.b $5A                   ; $8A83B2 static
    stz.b $78                   ; $8A83B4 static
    jsr.w $83BA                 ; $8A83B6 static
CODE_8A83B9:
    rts                         ; $8A83B9 static
    ldy.w #$0000                ; $8A83BA static
    ldx.b $78                   ; $8A83BD static
CODE_8A83BF:
    lda.l $7EBDF2,x             ; $8A83BF static
    sta.w $0812,y               ; $8A83C3 static
    lda.w #$FFFF                ; $8A83C6 static
    sta.w $0512,y               ; $8A83C9 static
    inx                         ; $8A83CC static
    inx                         ; $8A83CD static
    iny                         ; $8A83CE static
    iny                         ; $8A83CF static
    cpy.w #$0018                ; $8A83D0 static
    bne CODE_8A83BF             ; $8A83D3 static
    rts                         ; $8A83D5 static
    lda.b $38                   ; $8A83D6 static
    clc                         ; $8A83D8 static
    adc.b $56                   ; $8A83D9 static
    sta.b $38                   ; $8A83DB static
    inc.b $60                   ; $8A83DD static
    lda.b $60                   ; $8A83DF static
    and.w #$000F                ; $8A83E1 static
    bne CODE_8A83E8             ; $8A83E4 static
    inc.b $56                   ; $8A83E6 static
CODE_8A83E8:
    dec.b $46                   ; $8A83E8 static
    bne CODE_8A83F8             ; $8A83EA static
    lda.w #$8412                ; $8A83EC static
    sta.b $44                   ; $8A83EF static
    lda.w #$0010                ; $8A83F1 static
    sta.b $46                   ; $8A83F4 static
    inc.b $54                   ; $8A83F6 static
CODE_8A83F8:
    lda.b $4C                   ; $8A83F8 static
    sec                         ; $8A83FA static
    sbc.w #$FFB7                ; $8A83FB static
    cmp.w #$0015                ; $8A83FE static
    bpl CODE_8A8411             ; $8A8401 static
    asl a                       ; $8A8403 static
    asl a                       ; $8A8404 static
    asl a                       ; $8A8405 static
    sta.b $4E                   ; $8A8406 static
    asl a                       ; $8A8408 static
    clc                         ; $8A8409 static
    adc.b $4E                   ; $8A840A static
    sta.b $78                   ; $8A840C static
    jsr.w $83BA                 ; $8A840E static
CODE_8A8411:
    rts                         ; $8A8411 static
    inc.w $0160                 ; $8A8412 static
    lda.w $0160                 ; $8A8415 static
    and.w #$0003                ; $8A8418 static
    bne CODE_8A843C             ; $8A841B static
    inc.b $74                   ; $8A841D static
    dec.b $76                   ; $8A841F static
    dec.b $46                   ; $8A8421 static
    bne CODE_8A843C             ; $8A8423 static
    lda.w #$843D                ; $8A8425 static
    sta.b $44                   ; $8A8428 static
    lda.w #$0370                ; $8A842A static
    sta.b $46                   ; $8A842D static
    lda.w #$0006                ; $8A842F static
    sta.b $48                   ; $8A8432 static
    stz.b $56                   ; $8A8434 static
    stz.w $050A                 ; $8A8436 static
    stz.w $050C                 ; $8A8439 static
CODE_8A843C:
    rts                         ; $8A843C static
    jsr.w $8524                 ; $8A843D static
    lda.w #$0002                ; $8A8440 static
    sta.b $42                   ; $8A8443 static
    lda.b $3C                   ; $8A8445 static
    clc                         ; $8A8447 static
    adc.b $56                   ; $8A8448 static
    clc                         ; $8A844A static
    adc.b $56                   ; $8A844B static
    sta.b $3C                   ; $8A844D static
    inc.b $60                   ; $8A844F static
    lda.b $60                   ; $8A8451 static
    and.w #$0001                ; $8A8453 static
    bne CODE_8A84A9             ; $8A8456 static
    lda.b $56                   ; $8A8458 static
    cmp.w #$0063                ; $8A845A static
    bne CODE_8A8466             ; $8A845D static
    lda.w #$0003                ; $8A845F static
    sta.b $5A                   ; $8A8462 static
    inc.b $56                   ; $8A8464 static
CODE_8A8466:
    cmp.w #$0064                ; $8A8466 static
    bmi CODE_8A849F             ; $8A8469 static
    dec.b $62                   ; $8A846B static
    lda.b $34                   ; $8A846D static
    cmp.w #$0100                ; $8A846F static
    bmi CODE_8A8476             ; $8A8472 static
    dec.b $62                   ; $8A8474 static
CODE_8A8476:
    lda.w #$0001                ; $8A8476 static
    jsr.w $855A                 ; $8A8479 static
    lda.l $7E2C01               ; $8A847C static
    dec a                       ; $8A8480 static
    dec a                       ; $8A8481 static
    dec a                       ; $8A8482 static
    dec a                       ; $8A8483 static
    sta.l $7E2C01               ; $8A8484 static
    clc                         ; $8A8488 static
    adc.w #$0200                ; $8A8489 static
    sta.l $7E2C04               ; $8A848C static
    lda.b $34                   ; $8A8490 static
    cmp.w #$01E0                ; $8A8492 static
    bpl CODE_8A84A0             ; $8A8495 static
    clc                         ; $8A8497 static
    adc.w #$0008                ; $8A8498 static
    sta.b $34                   ; $8A849B static
    bra CODE_8A84A0             ; $8A849D static
CODE_8A849F:
    nop                         ; $8A849F static
CODE_8A84A0:
    lda.b $56                   ; $8A84A0 static
    cmp.w #$00AA                ; $8A84A2 static
    beq CODE_8A84A9             ; $8A84A5 static
    inc.b $56                   ; $8A84A7 static
CODE_8A84A9:
    dec.b $46                   ; $8A84A9 static
    bne CODE_8A84B8             ; $8A84AB static
    lda.w #$0046                ; $8A84AD static
    sta.b $46                   ; $8A84B0 static
    lda.w #$856D                ; $8A84B2 static
    sta.b $44                   ; $8A84B5 static
    rts                         ; $8A84B7 static
CODE_8A84B8:
    rts                         ; $8A84B8 static
objects_scatter_init:
    ldy.w #$FE84                ; $8A84B9 static
    ldx.w #$0000                ; $8A84BC static
CODE_8A84BF:
    lda.w #$FFFF                ; $8A84BF static
    sta.w $0510,x               ; $8A84C2 static
    sta.w $054C,x               ; $8A84C5 static
    sta.w $0588,x               ; $8A84C8 static
    lda.w #$0402                ; $8A84CB static
    sta.w $0810,x               ; $8A84CE static
    lda.w #$0403                ; $8A84D1 static
    sta.w $084C,x               ; $8A84D4 static
    sta.w $0888,x               ; $8A84D7 static
    tya                         ; $8A84DA static
    sta.w $0710,x               ; $8A84DB static
    sta.w $074C,x               ; $8A84DE static
    sta.w $0788,x               ; $8A84E1 static
    jsr.w $8506                 ; $8A84E4 static
    sta.w $0610,x               ; $8A84E7 static
    jsr.w $8506                 ; $8A84EA static
    sta.w $064C,x               ; $8A84ED static
    jsr.w $8506                 ; $8A84F0 static
    sta.w $0688,x               ; $8A84F3 static
    sta.b $88                   ; $8A84F6 static
    tya                         ; $8A84F8 static
    clc                         ; $8A84F9 static
    adc.w #$000F                ; $8A84FA static
    tay                         ; $8A84FD static
    inx                         ; $8A84FE static
    inx                         ; $8A84FF static
    cpx.w #$0020                ; $8A8500 static
    bne CODE_8A84BF             ; $8A8503 static
    rts                         ; $8A8505 static
    lda.b $12                   ; $8A8506 static
    bmi CODE_8A851D             ; $8A8508 static
    lda.b $10                   ; $8A850A static
    asl a                       ; $8A850C static
    eor.w #$2B45                ; $8A850D static
    sta.b $10                   ; $8A8510 static
    lda.b $12                   ; $8A8512 static
    rol a                       ; $8A8514 static
    eor.w #$1D87                ; $8A8515 static
    sta.b $12                   ; $8A8518 static
    lda.b $10                   ; $8A851A static
    rts                         ; $8A851C static
CODE_8A851D:
    asl.b $10                   ; $8A851D static
    rol.b $12                   ; $8A851F static
    lda.b $10                   ; $8A8521 static
    rts                         ; $8A8523 static
    ldx.w #$001E                ; $8A8524 static
CODE_8A8527:
    lda.b $8A                   ; $8A8527 static
    pha                         ; $8A8529 static
    lda.w $0610,x               ; $8A852A static
    clc                         ; $8A852D static
    adc.b $8A                   ; $8A852E static
    and.w #$00FF                ; $8A8530 static
    sta.w $0610,x               ; $8A8533 static
    dec.b $8A                   ; $8A8536 static
    lda.w $064C,x               ; $8A8538 static
    clc                         ; $8A853B static
    adc.b $8A                   ; $8A853C static
    and.w #$00FF                ; $8A853E static
    sta.w $064C,x               ; $8A8541 static
    dec.b $8A                   ; $8A8544 static
    lda.w $0688,x               ; $8A8546 static
    clc                         ; $8A8549 static
    adc.b $8A                   ; $8A854A static
    and.w #$00FF                ; $8A854C static
    sta.w $0688,x               ; $8A854F static
    pla                         ; $8A8552 static
    sta.b $8A                   ; $8A8553 static
    dex                         ; $8A8555 static
    dex                         ; $8A8556 static
    bpl CODE_8A8527             ; $8A8557 static
    rts                         ; $8A8559 static
    sta.b $7E                   ; $8A855A static
    ldx.w #$00DE                ; $8A855C static
CODE_8A855F:
    lda.w $0710,x               ; $8A855F static
    clc                         ; $8A8562 static
    adc.b $7E                   ; $8A8563 static
    sta.w $0710,x               ; $8A8565 static
    dex                         ; $8A8568 static
    dex                         ; $8A8569 static
    bpl CODE_8A855F             ; $8A856A static
    rts                         ; $8A856C static
    jsr.w $8524                 ; $8A856D static
    lda.b $34                   ; $8A8570 static
    beq CODE_8A8576             ; $8A8572 static
    bpl CODE_8A8580             ; $8A8574 static
CODE_8A8576:
    stz.b $34                   ; $8A8576 static
    lda.w #$0001                ; $8A8578 static
    sta.b $42                   ; $8A857B static
    jmp.w $8596                 ; $8A857D static
CODE_8A8580:
    lda.b $38                   ; $8A8580 static
    clc                         ; $8A8582 static
    adc.w #$0002                ; $8A8583 static
    sta.b $38                   ; $8A8586 static
    lda.b $36                   ; $8A8588 static
    clc                         ; $8A858A static
    adc.w #$0002                ; $8A858B static
    sta.b $36                   ; $8A858E static
    lda.w #$0001                ; $8A8590 static
    jsr.w $855A                 ; $8A8593 static
    dec.b $46                   ; $8A8596 static
    bne CODE_8A85A0             ; $8A8598 static
    lda.w #$8322                ; $8A859A static
    sta.b $44                   ; $8A859D static
    rts                         ; $8A859F static
CODE_8A85A0:
    lda.b $46                   ; $8A85A0 static
    cmp.w #$0032                ; $8A85A2 static
    bne CODE_8A85AE             ; $8A85A5 static
    stz.b $98                   ; $8A85A7 static
    lda.w #$FFC0                ; $8A85A9 static
    sta.b $96                   ; $8A85AC static
CODE_8A85AE:
    rts                         ; $8A85AE static
tile_anim_step:
    lda.b $42                   ; $8A85AF
    beq CODE_8A85CB             ; $8A85B1
    cmp.w #$0001                ; $8A85B3 static
    beq CODE_8A85D3             ; $8A85B6 static
    lda.b $40                   ; $8A85B8 static
    inc a                       ; $8A85BA static
    and.w #$000F                ; $8A85BB static
    sta.b $40                   ; $8A85BE static
    and.w #$0002                ; $8A85C0 static
    beq CODE_8A85CC             ; $8A85C3 static
    ldx.w #$0000                ; $8A85C5 static
    jsr.w anim_tiles_put        ; $8A85C8 static
CODE_8A85CB:
    rts                         ; $8A85CB
CODE_8A85CC:
    ldx.w #$001E                ; $8A85CC static
    jsr.w anim_tiles_put        ; $8A85CF static
    rts                         ; $8A85D2 static
CODE_8A85D3:
    lda.b $40                   ; $8A85D3 static
    inc a                       ; $8A85D5 static
    and.w #$0007                ; $8A85D6 static
    sta.b $40                   ; $8A85D9 static
    cmp.w #$0001                ; $8A85DB static
    beq CODE_8A85EB             ; $8A85DE static
    cmp.w #$0003                ; $8A85E0 static
    beq CODE_8A85F2             ; $8A85E3 static
    cmp.w #$0005                ; $8A85E5 static
    beq CODE_8A85F9             ; $8A85E8 static
    rts                         ; $8A85EA static
CODE_8A85EB:
    ldx.w #$005A                ; $8A85EB static
    jsr.w anim_tiles_put        ; $8A85EE static
    rts                         ; $8A85F1 static
CODE_8A85F2:
    ldx.w #$0078                ; $8A85F2 static
    jsr.w anim_tiles_put        ; $8A85F5 static
    rts                         ; $8A85F8 static
CODE_8A85F9:
    ldx.w #$003C                ; $8A85F9 static
    jsr.w anim_tiles_put        ; $8A85FC static
    rts                         ; $8A85FF static
anim_tiles_put:
    lda.w #$0496                ; $8A8600 static
    sta.w $2116                 ; $8A8603 static
    lda.l $7E8000,x             ; $8A8606 static
    sta.w $2118                 ; $8A860A static
    lda.l $7E8002,x             ; $8A860D static
    sta.w $2118                 ; $8A8611 static
    lda.l $7E8004,x             ; $8A8614 static
    sta.w $2118                 ; $8A8618 static
    lda.l $7E8006,x             ; $8A861B static
    sta.w $2118                 ; $8A861F static
    lda.l $7E8008,x             ; $8A8622 static
    sta.w $2118                 ; $8A8626 static
    lda.w #$0516                ; $8A8629 static
    sta.w $2116                 ; $8A862C static
    lda.l $7E800A,x             ; $8A862F static
    sta.w $2118                 ; $8A8633 static
    lda.l $7E800C,x             ; $8A8636 static
    sta.w $2118                 ; $8A863A static
    lda.l $7E800E,x             ; $8A863D static
    sta.w $2118                 ; $8A8641 static
    lda.l $7E8010,x             ; $8A8644 static
    sta.w $2118                 ; $8A8648 static
    lda.l $7E8012,x             ; $8A864B static
    sta.w $2118                 ; $8A864F static
    lda.w #$0596                ; $8A8652 static
    sta.w $2116                 ; $8A8655 static
    lda.l $7E8014,x             ; $8A8658 static
    sta.w $2118                 ; $8A865C static
    lda.l $7E8016,x             ; $8A865F static
    sta.w $2118                 ; $8A8663 static
    lda.l $7E8018,x             ; $8A8666 static
    sta.w $2118                 ; $8A866A static
    lda.l $7E801A,x             ; $8A866D static
    sta.w $2118                 ; $8A8671 static
    lda.l $7E801C,x             ; $8A8674 static
    sta.w $2118                 ; $8A8678 static
    rts                         ; $8A867B static
rotation_from_angle:
    lda.b $34                   ; $8A867C
    lsr a                       ; $8A867E
    lsr a                       ; $8A867F
    lsr a                       ; $8A8680
    and.w #$03FE                ; $8A8681
    tax                         ; $8A8684
    lda.l $81CEA9,x             ; $8A8685
    sta.b $14                   ; $8A8689
    sta.b $1A                   ; $8A868B
    lda.l $81CDA9,x             ; $8A868D
    sta.b $16                   ; $8A8691
    eor.w #$FFFF                ; $8A8693
    inc a                       ; $8A8696
    sta.b $18                   ; $8A8697
    rts                         ; $8A8699
func_8A869A:
    lda.b $3C                   ; $8A869A
    lsr a                       ; $8A869C
    lsr a                       ; $8A869D
    lsr a                       ; $8A869E
    lsr a                       ; $8A869F
    eor.w #$FFFF                ; $8A86A0
    inc a                       ; $8A86A3
    clc                         ; $8A86A4
    adc.w #$002A                ; $8A86A5
    sta.b $3A                   ; $8A86A8
    rts                         ; $8A86AA
scene_objects_place:
    lda.w $1536                 ; $8A86AB
    asl a                       ; $8A86AE
    asl a                       ; $8A86AF
    asl a                       ; $8A86B0
    tax                         ; $8A86B1
    lda.l $8A9351,x             ; $8A86B2
    and.w #$0001                ; $8A86B6
    bne CODE_8A86BC             ; $8A86B9
    rts                         ; $8A86BB
CODE_8A86BC:
    lda.b $3C                   ; $8A86BC
    lsr a                       ; $8A86BE
    lsr a                       ; $8A86BF
    lsr a                       ; $8A86C0
    lsr a                       ; $8A86C1
    lsr a                       ; $8A86C2
    lsr a                       ; $8A86C3
    sta.b $7E                   ; $8A86C4
    lsr a                       ; $8A86C6
    sta.b $80                   ; $8A86C7
    sta.b $82                   ; $8A86C9
    lda.b $62                   ; $8A86CB
    eor.w #$FFFF                ; $8A86CD
    inc a                       ; $8A86D0
    sta.b $84                   ; $8A86D1
    sta.b $86                   ; $8A86D3
    sta.b $88                   ; $8A86D5
    ldx.w #$0000                ; $8A86D7
    clc                         ; $8A86DA
    lda.b $84                   ; $8A86DB
    adc.w #$0010                ; $8A86DD
    sta.b $7C                   ; $8A86E0
    lda.b $7E                   ; $8A86E2
    adc.w #$0014                ; $8A86E4
    sta.b $7A                   ; $8A86E7
    jsr.w func_8A9202           ; $8A86E9
    clc                         ; $8A86EC
    lda.b $84                   ; $8A86ED
    adc.w #$0020                ; $8A86EF
    sta.b $7C                   ; $8A86F2
    lda.b $7E                   ; $8A86F4
    adc.w #$00C8                ; $8A86F6
    sta.b $7A                   ; $8A86F9
    jsr.w func_8A9202           ; $8A86FB
    clc                         ; $8A86FE
    lda.b $84                   ; $8A86FF
    sta.b $7C                   ; $8A8701
    lda.b $80                   ; $8A8703
    adc.w #$000A                ; $8A8705
    sta.b $7A                   ; $8A8708
    jsr.w func_8A92CF           ; $8A870A
    clc                         ; $8A870D
    lda.b $84                   ; $8A870E
    adc.w #$0032                ; $8A8710
    sta.b $7C                   ; $8A8713
    lda.b $82                   ; $8A8715
    adc.w #$0012                ; $8A8717
    sta.b $7A                   ; $8A871A
    jsr.w func_8A92CF           ; $8A871C
    clc                         ; $8A871F
    lda.b $84                   ; $8A8720
    adc.w #$0028                ; $8A8722
    sta.b $7C                   ; $8A8725
    lda.b $80                   ; $8A8727
    adc.w #$00F0                ; $8A8729
    sta.b $7A                   ; $8A872C
    jsr.w func_8A92CF           ; $8A872E
    clc                         ; $8A8731
    lda.b $84                   ; $8A8732
    adc.w #$0032                ; $8A8734
    sta.b $7C                   ; $8A8737
    lda.b $82                   ; $8A8739
    adc.w #$00C8                ; $8A873B
    sta.b $7A                   ; $8A873E
    jsr.w func_8A92CF           ; $8A8740
    clc                         ; $8A8743
    lda.b $84                   ; $8A8744
    adc.w #$0019                ; $8A8746
    sta.b $7C                   ; $8A8749
    lda.b $82                   ; $8A874B
    adc.w #$0080                ; $8A874D
    sta.b $7A                   ; $8A8750
    jsr.w func_8A92CF           ; $8A8752
    clc                         ; $8A8755
    lda.b $84                   ; $8A8756
    sbc.w #$000A                ; $8A8758
    sta.b $7C                   ; $8A875B
    lda.b $80                   ; $8A875D
    adc.w #$0140                ; $8A875F
    sta.b $7A                   ; $8A8762
    jsr.w func_8A92CF           ; $8A8764
    rts                         ; $8A8767
track_preview_maybe:
    lda.w $1536                 ; $8A8768
    asl a                       ; $8A876B
    asl a                       ; $8A876C
    asl a                       ; $8A876D
    tax                         ; $8A876E
    lda.l $8A9351,x             ; $8A876F
    and.w #$0002                ; $8A8773
    bne CODE_8A8779             ; $8A8776
    rts                         ; $8A8778
CODE_8A8779:
    phb                         ; $8A8779
    pea.w $8383                 ; $8A877A
    plb                         ; $8A877D
    plb                         ; $8A877E
    lda.w $1536                 ; $8A877F
    jsr.w track_tiles_preview   ; $8A8782
    plb                         ; $8A8785
    rts                         ; $8A8786
hdma_title_setup:
    lda.w #$0500                ; $8A8787
    sta.w $4310                 ; $8A878A
    lda.w #$88DE                ; $8A878D
    sta.w $4312                 ; $8A8790
    sep.b #$20                  ; $8A8793
    lda.b #$8A                  ; $8A8795
    sta.w $4314                 ; $8A8797
    rep.b #$20                  ; $8A879A
    lda.w #$2C00                ; $8A879C
    sta.w $4320                 ; $8A879F
    lda.w #$88E7                ; $8A87A2
    sta.w $4322                 ; $8A87A5
    sep.b #$20                  ; $8A87A8
    lda.b #$8A                  ; $8A87AA
    sta.w $4324                 ; $8A87AC
    rep.b #$20                  ; $8A87AF
    lda.w #$0B00                ; $8A87B1
    sta.w $4340                 ; $8A87B4
    lda.w #$88F0                ; $8A87B7
    sta.w $4342                 ; $8A87BA
    sep.b #$20                  ; $8A87BD
    lda.b #$8A                  ; $8A87BF
    sta.w $4344                 ; $8A87C1
    rep.b #$20                  ; $8A87C4
    lda.w #$0700                ; $8A87C6
    sta.w $4330                 ; $8A87C9
    lda.w #$88F9                ; $8A87CC
    sta.w $4332                 ; $8A87CF
    sep.b #$20                  ; $8A87D2
    lda.b #$8A                  ; $8A87D4
    sta.w $4334                 ; $8A87D6
    rep.b #$20                  ; $8A87D9
    phb                         ; $8A87DB
    ldx.w #$8902                ; $8A87DC
    ldy.w #$0D00                ; $8A87DF
    lda.w #$000C                ; $8A87E2
    mvn $81,$8A                 ; $8A87E5
    plb                         ; $8A87E8
    lda.w #$0E02                ; $8A87E9
    sta.w $4350                 ; $8A87EC
    lda.w #$0D00                ; $8A87EF
    sta.w $4352                 ; $8A87F2
    sep.b #$20                  ; $8A87F5
    lda.b #$81                  ; $8A87F7
    sta.w $4354                 ; $8A87F9
    sta.w $4357                 ; $8A87FC
    rep.b #$20                  ; $8A87FF
    phb                         ; $8A8801
    ldx.w #$890F                ; $8A8802
    ldy.w #$0D20                ; $8A8805
    lda.w #$000C                ; $8A8808
    mvn $81,$8A                 ; $8A880B
    plb                         ; $8A880E
    lda.w #$0D02                ; $8A880F
    sta.w $4370                 ; $8A8812
    lda.w #$0D20                ; $8A8815
    sta.w $4372                 ; $8A8818
    sep.b #$20                  ; $8A881B
    lda.b #$A1                  ; $8A881D
    sta.w $4374                 ; $8A881F
    sta.w $4377                 ; $8A8822
    rep.b #$20                  ; $8A8825
    rts                         ; $8A8827
vram_planes_build:
    ldy.w #$1900                ; $8A8828
    sty.w $4300                 ; $8A882B
    ldy.w #$8633                ; $8A882E
    sty.w $4302                 ; $8A8831
    lda.w #$1200                ; $8A8834
    sta.w $4305                 ; $8A8837
    lda.w #$0000                ; $8A883A
    sta.w $2116                 ; $8A883D
    sep.b #$20                  ; $8A8840
    lda.b #$7E                  ; $8A8842
    sta.w $4304                 ; $8A8844
    lda.b #$01                  ; $8A8847
    sta.w $420B                 ; $8A8849
    rep.b #$20                  ; $8A884C
    ldy.w #$1900                ; $8A884E
    sty.w $4300                 ; $8A8851
    ldy.w #$9833                ; $8A8854
    sty.w $4302                 ; $8A8857
    lda.w #$06C0                ; $8A885A
    sta.w $4305                 ; $8A885D
    lda.w #$1200                ; $8A8860
    sta.w $2116                 ; $8A8863
    sep.b #$20                  ; $8A8866
    lda.b #$7E                  ; $8A8868
    sta.w $4304                 ; $8A886A
    lda.b #$01                  ; $8A886D
    sta.w $420B                 ; $8A886F
    rep.b #$20                  ; $8A8872
    ldy.w #$1808                ; $8A8874
    sty.w $4300                 ; $8A8877
    ldy.w #$88DD                ; $8A887A
    sty.w $4302                 ; $8A887D
    lda.w #$4000                ; $8A8880
    sta.w $4305                 ; $8A8883
    lda.w #$0000                ; $8A8886
    sta.w $2116                 ; $8A8889
    sep.b #$20                  ; $8A888C
    stz.w $2115                 ; $8A888E
    lda.b #$8A                  ; $8A8891
    sta.w $4304                 ; $8A8893
    lda.b #$01                  ; $8A8896
    sta.w $420B                 ; $8A8898
    rep.b #$20                  ; $8A889B
    lda.w #$1800                ; $8A889D
    sta.w $4300                 ; $8A88A0
    lda.w #$80BA                ; $8A88A3
    sta.w $4302                 ; $8A88A6
    sep.b #$20                  ; $8A88A9
    lda.b #$7E                  ; $8A88AB
    sta.w $4304                 ; $8A88AD
    rep.b #$20                  ; $8A88B0
    ldx.w #$0006                ; $8A88B2
    ldy.w #$000F                ; $8A88B5
    lda.w #$0308                ; $8A88B8
CODE_8A88BB:
    sty.w $4305                 ; $8A88BB
    sta.w $2116                 ; $8A88BE
    sep.b #$20                  ; $8A88C1
    pha                         ; $8A88C3
    lda.b #$01                  ; $8A88C4
    sta.w $420B                 ; $8A88C6
    pla                         ; $8A88C9
    rep.b #$20                  ; $8A88CA
    clc                         ; $8A88CC
    adc.w #$0080                ; $8A88CD
    dex                         ; $8A88D0
    bne CODE_8A88BB             ; $8A88D1
    sep.b #$20                  ; $8A88D3
    lda.b #$80                  ; $8A88D5
    sta.w $2115                 ; $8A88D7
    rep.b #$20                  ; $8A88DA
    rts                         ; $8A88DC
    incbin !baserom:$508DD..$5091C
title_scene_load:
    lda.w #$007E                ; $8A891C
    ldx.w #$8256                ; $8A891F
    ldy.w #$2000                ; $8A8922
    jsl.l decompress_to_vram_long; $8A8925
    lda.w #$007E                ; $8A8929
    ldx.w #$B385                ; $8A892C
    ldy.w #$2A00                ; $8A892F
    jsl.l decompress_to_vram_long; $8A8932
    lda.w #$007E                ; $8A8936
    ldx.w #$9F07                ; $8A8939
    ldy.w #$3000                ; $8A893C
    jsl.l decompress_to_vram_long; $8A893F
    lda.w #$007E                ; $8A8943
    sta.b $14                   ; $8A8946
    ldx.w #$A071                ; $8A8948
    ldy.w #$3600                ; $8A894B
    lda.w #$0800                ; $8A894E
    jsl.l upload_vram_now       ; $8A8951
    stz.b $5A                   ; $8A8955
    ldx.w #$0000                ; $8A8957
    ldy.w #$0200                ; $8A895A
    stx.b $3C                   ; $8A895D
    sty.b $3E                   ; $8A895F
    jsr.w screen_objects_place  ; $8A8961
    lda.w #$0003                ; $8A8964
    sta.b $36                   ; $8A8967
    lsr a                       ; $8A8969
    lsr a                       ; $8A896A
    sta.b $4A                   ; $8A896B
    lda.w #$0000                ; $8A896D
    sta.b $38                   ; $8A8970
    lsr a                       ; $8A8972
    lsr a                       ; $8A8973
    sta.b $4C                   ; $8A8974
    lda.w #$FF36                ; $8A8976
    sta.w $0600                 ; $8A8979
    lda.w #$009B                ; $8A897C
    sta.w $0700                 ; $8A897F
    lda.w #$0100                ; $8A8982
    sta.w $0800                 ; $8A8985
    lda.w #$0001                ; $8A8988
    sta.w $0500                 ; $8A898B
    rts                         ; $8A898E
screen_objects_place:
    lda.b $5E                   ; $8A898F
    beq CODE_8A8994             ; $8A8991
    rts                         ; $8A8993 static
CODE_8A8994:
    lda.b $3C                   ; $8A8994
    pha                         ; $8A8996
    lsr a                       ; $8A8997
    lsr a                       ; $8A8998
    sec                         ; $8A8999
    sbc.w #$0100                ; $8A899A
    sta.b $3C                   ; $8A899D
    cmp.w #$0120                ; $8A899F
    bmi CODE_8A89AD             ; $8A89A2
    pla                         ; $8A89A4 static
    sta.b $3C                   ; $8A89A5 static
    inc.b $5E                   ; $8A89A7 static
    jsr.w objects_scatter_init  ; $8A89A9 static
    rts                         ; $8A89AC static
CODE_8A89AD:
    lda.b $3C                   ; $8A89AD
    clc                         ; $8A89AF
    adc.w #$0030                ; $8A89B0
    sta.w $0602                 ; $8A89B3
    sta.w $0606                 ; $8A89B6
    clc                         ; $8A89B9
    adc.w #$0010                ; $8A89BA
    sta.w $0604                 ; $8A89BD
    sta.w $0608                 ; $8A89C0
    lda.b $3E                   ; $8A89C3
    pha                         ; $8A89C5
    lsr a                       ; $8A89C6
    lsr a                       ; $8A89C7
    sta.b $3E                   ; $8A89C8
    clc                         ; $8A89CA
    adc.w #$0008                ; $8A89CB
    sta.w $0702                 ; $8A89CE
    sta.w $0704                 ; $8A89D1
    clc                         ; $8A89D4
    adc.w #$0010                ; $8A89D5
    sta.w $0706                 ; $8A89D8
    sta.w $0708                 ; $8A89DB
    jmp.w $8A43                 ; $8A89DE
    incbin !baserom:$509E1..$50A43
    ldx.w #$0000                ; $8A8A43
    ldy.w #$0006                ; $8A8A46
CODE_8A8A49:
    jsr.w table_062a_fill       ; $8A8A49
    lda.b $3E                   ; $8A8A4C
    clc                         ; $8A8A4E
    adc.w #$0010                ; $8A8A4F
    sta.b $3E                   ; $8A8A52
    dey                         ; $8A8A54
    bne CODE_8A8A49             ; $8A8A55
    ldx.w #$0000                ; $8A8A57
    ldy.w #$0048                ; $8A8A5A
CODE_8A8A5D:
    lda.l $7E8152,x             ; $8A8A5D
    cmp.w #$0004                ; $8A8A61
    beq CODE_8A8A77             ; $8A8A64
    cpy.w #$0032                ; $8A8A66
    bpl CODE_8A8A6E             ; $8A8A69
    ora.w #$3000                ; $8A8A6B
CODE_8A8A6E:
    sta.w $082A,x               ; $8A8A6E
    lda.w #$0001                ; $8A8A71
    sta.w $052A,x               ; $8A8A74
CODE_8A8A77:
    inx                         ; $8A8A77
    inx                         ; $8A8A78
    dey                         ; $8A8A79
    bne CODE_8A8A5D             ; $8A8A7A
    pla                         ; $8A8A7C
    sta.b $3E                   ; $8A8A7D
    pla                         ; $8A8A7F
    sta.b $3C                   ; $8A8A80
    clc                         ; $8A8A82
    lda.w $072A                 ; $8A8A83
    adc.w #$0010                ; $8A8A86
    sta.w $0712                 ; $8A8A89
    sta.w $0714                 ; $8A8A8C
    sta.w $0716                 ; $8A8A8F
    sta.w $0718                 ; $8A8A92
    adc.w #$0008                ; $8A8A95
    sta.w $071A                 ; $8A8A98
    sta.w $071C                 ; $8A8A9B
    sta.w $071E                 ; $8A8A9E
    sta.w $0720                 ; $8A8AA1
    adc.w #$0008                ; $8A8AA4
    sta.w $0722                 ; $8A8AA7
    sta.w $0724                 ; $8A8AAA
    sta.w $0726                 ; $8A8AAD
    sta.w $0728                 ; $8A8AB0
    lda.w $062A                 ; $8A8AB3
    adc.w #$0050                ; $8A8AB6
    sta.w $0612                 ; $8A8AB9
    sta.w $061A                 ; $8A8ABC
    sta.w $0622                 ; $8A8ABF
    adc.w #$0008                ; $8A8AC2
    sta.w $0614                 ; $8A8AC5
    sta.w $061C                 ; $8A8AC8
    sta.w $0624                 ; $8A8ACB
    adc.w #$0008                ; $8A8ACE
    sta.w $0616                 ; $8A8AD1
    sta.w $061E                 ; $8A8AD4
    sta.w $0626                 ; $8A8AD7
    adc.w #$0008                ; $8A8ADA
    sta.w $0618                 ; $8A8ADD
    sta.w $0620                 ; $8A8AE0
    sta.w $0628                 ; $8A8AE3
    rts                         ; $8A8AE6
table_062a_fill:
    phy                         ; $8A8AE7
    lda.b $3C                   ; $8A8AE8
    pha                         ; $8A8AEA
    ldy.w #$000C                ; $8A8AEB
CODE_8A8AEE:
    lda.b $3C                   ; $8A8AEE
    sta.w $062A,x               ; $8A8AF0
    lda.b $3E                   ; $8A8AF3
    sta.w $072A,x               ; $8A8AF5
    inx                         ; $8A8AF8
    inx                         ; $8A8AF9
    lda.b $3C                   ; $8A8AFA
    clc                         ; $8A8AFC
    adc.w #$0010                ; $8A8AFD
    sta.b $3C                   ; $8A8B00
    dey                         ; $8A8B02
    bne CODE_8A8AEE             ; $8A8B03
    pla                         ; $8A8B05
    sta.b $3C                   ; $8A8B06
    ply                         ; $8A8B08
    rts                         ; $8A8B09
mode7_view:
    lda.b $4C                   ; $8A8B0A
    sta.w $0D01                 ; $8A8B0C
    sta.w $0D04                 ; $8A8B0F
    sta.w $0D07                 ; $8A8B12
    lda.b $4A                   ; $8A8B15
    sta.w $0D21                 ; $8A8B17
    sta.w $0D24                 ; $8A8B1A
    sta.w $0D27                 ; $8A8B1D
    sep.b #$20                  ; $8A8B20
    lda.b $14                   ; $8A8B22
    sta.w $211B                 ; $8A8B24
    lda.b $15                   ; $8A8B27
    sta.w $211B                 ; $8A8B29
    lda.b $16                   ; $8A8B2C
    sta.w $211C                 ; $8A8B2E
    lda.b $17                   ; $8A8B31
    sta.w $211C                 ; $8A8B33
    lda.b $18                   ; $8A8B36
    sta.w $211D                 ; $8A8B38
    lda.b $19                   ; $8A8B3B
    sta.w $211D                 ; $8A8B3D
    lda.b $1A                   ; $8A8B40
    sta.w $211E                 ; $8A8B42
    lda.b $1B                   ; $8A8B45
    sta.w $211E                 ; $8A8B47
    lda.b #$80                  ; $8A8B4A
    sta.w $211F                 ; $8A8B4C
    lda.b #$00                  ; $8A8B4F
    sta.w $211F                 ; $8A8B51
    lda.b #$40                  ; $8A8B54
    sta.w $2120                 ; $8A8B56
    lda.b #$00                  ; $8A8B59
    sta.w $2120                 ; $8A8B5B
    lda.b $3A                   ; $8A8B5E
    sta.w $210F                 ; $8A8B60
    lda.b $3B                   ; $8A8B63
    sta.w $210F                 ; $8A8B65
    rep.b #$20                  ; $8A8B68
    lda.b $3A                   ; $8A8B6A
    clc                         ; $8A8B6C
    adc.w #$0200                ; $8A8B6D
    asl a                       ; $8A8B70
    sta.w $0D2A                 ; $8A8B71
    lda.w $1536                 ; $8A8B74
    asl a                       ; $8A8B77
    asl a                       ; $8A8B78
    asl a                       ; $8A8B79
    tax                         ; $8A8B7A
    lda.l $8A9351,x             ; $8A8B7B
    and.w #$0004                ; $8A8B7F
    bne CODE_8A8B9D             ; $8A8B82
    sep.b #$20                  ; $8A8B84
    lda.b #$80                  ; $8A8B86
    sta.w $2110                 ; $8A8B88
    lda.b #$00                  ; $8A8B8B
    sta.w $2110                 ; $8A8B8D
    rep.b #$20                  ; $8A8B90
    lda.b $62                   ; $8A8B92
    cmp.w #$FFC0                ; $8A8B94
    bmi CODE_8A8B9C             ; $8A8B97
    sta.w $0D0A                 ; $8A8B99
CODE_8A8B9C:
    rts                         ; $8A8B9C
CODE_8A8B9D:
    lda.b $62                   ; $8A8B9D static
    cmp.w #$FFC0                ; $8A8B9F static
    bmi CODE_8A8BB5             ; $8A8BA2 static
    sta.w $0D0A                 ; $8A8BA4 static
    sep.b #$20                  ; $8A8BA7 static
    lda.b $62                   ; $8A8BA9 static
    sta.w $2110                 ; $8A8BAB static
    lda.b $63                   ; $8A8BAE static
    sta.w $2110                 ; $8A8BB0 static
    rep.b #$20                  ; $8A8BB3 static
CODE_8A8BB5:
    rts                         ; $8A8BB5 static
wait_frames_8a8bb6:
    clc                         ; $8A8BB6
    adc.l $810024               ; $8A8BB7
CODE_8A8BBB:
    cmp.l $810024               ; $8A8BBB
    bne CODE_8A8BBB             ; $8A8BBF
    rtl                         ; $8A8BC1
track_tiles_preview:
    tax                         ; $8A8BC2
    lda.w $8410,x               ; $8A8BC3
    and.w #$00FF                ; $8A8BC6
    tax                         ; $8A8BC9
    lda.w $8447,x               ; $8A8BCA
    asl a                       ; $8A8BCD
    tay                         ; $8A8BCE
    phx                         ; $8A8BCF
    tyx                         ; $8A8BD0
    lda.l $838585,x             ; $8A8BD1
    pha                         ; $8A8BD5
    lda.l $838597,x             ; $8A8BD6
    ldy.w #$0000                ; $8A8BDA
    plx                         ; $8A8BDD
    jsl.l decompress            ; $8A8BDE
    plx                         ; $8A8BE2
    phx                         ; $8A8BE3
    lda.l $810444               ; $8A8BE4
    asl a                       ; $8A8BE8
    tax                         ; $8A8BE9
    jmp.w ($8BED,x)             ; $8A8BEA
    incbin !baserom:$50BED..$50C5B
    plx                         ; $8A8C5B
    lda.w #$0420                ; $8A8C5C
    jsr.w tiles_planar_upload   ; $8A8C5F
    lda.w #$0000                ; $8A8C62
    jsr.w upload_tiles_c280     ; $8A8C65
    lda.w #$0200                ; $8A8C68
    jsr.w upload_tiles_c680     ; $8A8C6B
    jsr.w palette_load_70       ; $8A8C6E
    rts                         ; $8A8C71
    plx                         ; $8A8C72
    lda.w #$0820                ; $8A8C73
    jsr.w tiles_planar_upload   ; $8A8C76
    lda.w #$0000                ; $8A8C79
    jsr.w upload_tiles_c280     ; $8A8C7C
    lda.w #$0200                ; $8A8C7F
    jsr.w upload_tiles_c680     ; $8A8C82
    jsr.w palette_load_70       ; $8A8C85
    rts                         ; $8A8C88
    plx                         ; $8A8C89
    lda.w #$0440                ; $8A8C8A
    jsr.w tiles_planar_upload   ; $8A8C8D
    lda.w #$0020                ; $8A8C90
    jsr.w upload_tiles_c280     ; $8A8C93
    lda.w #$0220                ; $8A8C96
    jsr.w upload_tiles_c680     ; $8A8C99
    jsr.w palette_load_70       ; $8A8C9C
    rts                         ; $8A8C9F
    plx                         ; $8A8CA0
    lda.w #$0840                ; $8A8CA1
    jsr.w tiles_planar_upload   ; $8A8CA4
    lda.w #$0020                ; $8A8CA7
    jsr.w upload_tiles_c280     ; $8A8CAA
    lda.w #$0220                ; $8A8CAD
    jsr.w upload_tiles_c680     ; $8A8CB0
    jsr.w palette_load_70       ; $8A8CB3
    rts                         ; $8A8CB6
tiles_planar_upload:
    sta.b $38                   ; $8A8CB7
    phx                         ; $8A8CB9
    lda.w $851B,x               ; $8A8CBA
    lsr a                       ; $8A8CBD
    lsr a                       ; $8A8CBE
    lsr a                       ; $8A8CBF
    lsr a                       ; $8A8CC0
    lsr a                       ; $8A8CC1
    sta.b $3C                   ; $8A8CC2
    lda.w $84B1,x               ; $8A8CC4
    clc                         ; $8A8CC7
    adc.b $38                   ; $8A8CC8
    tay                         ; $8A8CCA
    pea.w $837F                 ; $8A8CCB
    plb                         ; $8A8CCE
    ldx.w #$0000                ; $8A8CCF
    sep.b #$20                  ; $8A8CD2
CODE_8A8CD4:
    lda.b #$04                  ; $8A8CD4
    sta.b $3A                   ; $8A8CD6
CODE_8A8CD8:
    lda.w $0000,y               ; $8A8CD8
    asl a                       ; $8A8CDB
    rol.b $25,x                 ; $8A8CDC
    asl a                       ; $8A8CDE
    rol.b $24,x                 ; $8A8CDF
    asl a                       ; $8A8CE1
    rol.b $15,x                 ; $8A8CE2
    asl a                       ; $8A8CE4
    rol.b $14,x                 ; $8A8CE5
    asl a                       ; $8A8CE7
    rol.b $25,x                 ; $8A8CE8
    asl a                       ; $8A8CEA
    rol.b $24,x                 ; $8A8CEB
    asl a                       ; $8A8CED
    rol.b $15,x                 ; $8A8CEE
    asl a                       ; $8A8CF0
    rol.b $14,x                 ; $8A8CF1
    iny                         ; $8A8CF3
    dec.b $3A                   ; $8A8CF4
    bne CODE_8A8CD8             ; $8A8CF6
    inx                         ; $8A8CF8
    inx                         ; $8A8CF9
    cpx.w #$0010                ; $8A8CFA
    bne CODE_8A8CD4             ; $8A8CFD
    rep.b #$20                  ; $8A8CFF
    tya                         ; $8A8D01
    sec                         ; $8A8D02
    sbc.w #$0020                ; $8A8D03
    tay                         ; $8A8D06
    lda.b $14                   ; $8A8D07
    sta.w $0000,y               ; $8A8D09
    lda.b $16                   ; $8A8D0C
    sta.w $0002,y               ; $8A8D0E
    lda.b $18                   ; $8A8D11
    sta.w $0004,y               ; $8A8D13
    lda.b $1A                   ; $8A8D16
    sta.w $0006,y               ; $8A8D18
    lda.b $1C                   ; $8A8D1B
    sta.w $0008,y               ; $8A8D1D
    lda.b $1E                   ; $8A8D20
    sta.w $000A,y               ; $8A8D22
    lda.b $20                   ; $8A8D25
    sta.w $000C,y               ; $8A8D27
    lda.b $22                   ; $8A8D2A
    sta.w $000E,y               ; $8A8D2C
    lda.b $24                   ; $8A8D2F
    sta.w $0010,y               ; $8A8D31
    lda.b $26                   ; $8A8D34
    sta.w $0012,y               ; $8A8D36
    lda.b $28                   ; $8A8D39
    sta.w $0014,y               ; $8A8D3B
    lda.b $2A                   ; $8A8D3E
    sta.w $0016,y               ; $8A8D40
    lda.b $2C                   ; $8A8D43
    sta.w $0018,y               ; $8A8D45
    lda.b $2E                   ; $8A8D48
    sta.w $001A,y               ; $8A8D4A
    lda.b $30                   ; $8A8D4D
    sta.w $001C,y               ; $8A8D4F
    lda.b $32                   ; $8A8D52
    sta.w $001E,y               ; $8A8D54
    tya                         ; $8A8D57
    clc                         ; $8A8D58
    adc.w #$0020                ; $8A8D59
    tay                         ; $8A8D5C
    dec.b $3C                   ; $8A8D5D
    beq CODE_8A8D64             ; $8A8D5F
    jmp.w $8CCF                 ; $8A8D61
CODE_8A8D64:
    plb                         ; $8A8D64
    plx                         ; $8A8D65
    phx                         ; $8A8D66
    lda.w #$007F                ; $8A8D67
    sta.b $14                   ; $8A8D6A
    lda.w $851B,x               ; $8A8D6C
    pha                         ; $8A8D6F
    lda.w $84B1,x               ; $8A8D70
    clc                         ; $8A8D73
    adc.b $38                   ; $8A8D74
    tax                         ; $8A8D76
    pla                         ; $8A8D77
    ldy.w #$6FE0                ; $8A8D78
    jsl.l upload_vram_now       ; $8A8D7B
    plx                         ; $8A8D7F
    rts                         ; $8A8D80
upload_tiles_c280:
    phx                         ; $8A8D81
    sta.b $16                   ; $8A8D82
    lda.w #$007F                ; $8A8D84
    sta.b $14                   ; $8A8D87
    lda.w $84B1,x               ; $8A8D89
    clc                         ; $8A8D8C
    adc.w #$0020                ; $8A8D8D
    clc                         ; $8A8D90
    adc.b $16                   ; $8A8D91
    tax                         ; $8A8D93
    lda.w #$0200                ; $8A8D94
    ldy.w #$C280                ; $8A8D97
    jsl.l upload_vram_now       ; $8A8D9A
    plx                         ; $8A8D9E
    rts                         ; $8A8D9F
upload_tiles_c680:
    phx                         ; $8A8DA0
    sta.b $16                   ; $8A8DA1
    lda.w #$007F                ; $8A8DA3
    sta.b $14                   ; $8A8DA6
    lda.w $84B1,x               ; $8A8DA8
    clc                         ; $8A8DAB
    adc.w #$0020                ; $8A8DAC
    clc                         ; $8A8DAF
    adc.b $16                   ; $8A8DB0
    tax                         ; $8A8DB2
    lda.w #$0200                ; $8A8DB3
    ldy.w #$C680                ; $8A8DB6
    jsl.l upload_vram_now       ; $8A8DB9
    plx                         ; $8A8DBD
    rts                         ; $8A8DBE
palette_load_70:
    phx                         ; $8A8DBF
    lda.l $8384B1,x             ; $8A8DC0
    tax                         ; $8A8DC4
    sep.b #$20                  ; $8A8DC5
    stz.w $2121                 ; $8A8DC7
    lda.l $7F0000,x             ; $8A8DCA
    sta.w $2122                 ; $8A8DCE
    lda.l $7F0001,x             ; $8A8DD1
    sta.w $2122                 ; $8A8DD5
    rep.b #$20                  ; $8A8DD8
    lda.l $7F0000,x             ; $8A8DDA
    sta.b $72                   ; $8A8DDE
    lda.w #$7F70                ; $8A8DE0
    ldy.w #$0020                ; $8A8DE3
    jsl.l upload_cgram_now      ; $8A8DE6
    plx                         ; $8A8DEA
    rts                         ; $8A8DEB
clear_vram:
    sep.b #$20                  ; $8A8DEC
    rep.b #$10                  ; $8A8DEE
    ldx.w #$0000                ; $8A8DF0
    stx.w $2116                 ; $8A8DF3
    lda.b #$09                  ; $8A8DF6
    sta.w $4300                 ; $8A8DF8
    lda.b #$18                  ; $8A8DFB
    sta.w $4301                 ; $8A8DFD
    lda.b #$8A                  ; $8A8E00
    sta.w $4304                 ; $8A8E02
    ldx.w #$8E19                ; $8A8E05
    stx.w $4302                 ; $8A8E08
    ldx.w #$0000                ; $8A8E0B
    stx.w $4305                 ; $8A8E0E
    lda.b #$01                  ; $8A8E11
    sta.w $420B                 ; $8A8E13
    rep.b #$20                  ; $8A8E16
    rtl                         ; $8A8E18
    incbin !baserom:$50E19..$50E1B
oam_from_objects:
    rep.b #$30                  ; $8A8E1B
    stz.w $0B00                 ; $8A8E1D
    stz.w $0B02                 ; $8A8E20
    stz.w $0B04                 ; $8A8E23
    stz.w $0B06                 ; $8A8E26
    stz.w $0B08                 ; $8A8E29
    stz.w $0B0A                 ; $8A8E2C
    stz.w $0B0C                 ; $8A8E2F
    stz.w $0B0E                 ; $8A8E32
    stz.w $0B10                 ; $8A8E35
    stz.w $0B12                 ; $8A8E38
    stz.w $0B14                 ; $8A8E3B
    stz.w $0B16                 ; $8A8E3E
    stz.w $0B18                 ; $8A8E41
    stz.w $0B1A                 ; $8A8E44
    stz.w $0B1C                 ; $8A8E47
    stz.w $0B1E                 ; $8A8E4A
    stz.b $28                   ; $8A8E4D
    stz.b $2A                   ; $8A8E4F
    lda.w #$0001                ; $8A8E51
    sta.b $2C                   ; $8A8E54
    lda.w #$0002                ; $8A8E56
    sta.b $2E                   ; $8A8E59
    ldx.w #$0000                ; $8A8E5B
CODE_8A8E5E:
    lda.w $0500,x               ; $8A8E5E
    bne CODE_8A8E86             ; $8A8E61
CODE_8A8E63:
    inx                         ; $8A8E63
    inx                         ; $8A8E64
    cpx.w #$0100                ; $8A8E65
    bne CODE_8A8E5E             ; $8A8E68
    lda.b $2A                   ; $8A8E6A
    cmp.b $30                   ; $8A8E6C
    beq CODE_8A8E81             ; $8A8E6E
    bpl CODE_8A8E81             ; $8A8E70
    tay                         ; $8A8E72
    lda.w #$E000                ; $8A8E73
CODE_8A8E76:
    sta.w $0900,y               ; $8A8E76
    iny                         ; $8A8E79
    iny                         ; $8A8E7A
    iny                         ; $8A8E7B
    iny                         ; $8A8E7C
    cpy.b $30                   ; $8A8E7D
    bne CODE_8A8E76             ; $8A8E7F
CODE_8A8E81:
    ldy.b $2A                   ; $8A8E81
    sty.b $30                   ; $8A8E83
    rtl                         ; $8A8E85
CODE_8A8E86:
    lda.w $0600,x               ; $8A8E86
    and.w #$01FF                ; $8A8E89
    sta.b $32                   ; $8A8E8C
    lda.w $0700,x               ; $8A8E8E
    bmi CODE_8A8E63             ; $8A8E91
    cmp.w #$00F0                ; $8A8E93
    bpl CODE_8A8E63             ; $8A8E96
    sta.b $26                   ; $8A8E98
    ldy.b $28                   ; $8A8E9A
    lda.b $32                   ; $8A8E9C
    and.w #$FF00                ; $8A8E9E
    beq CODE_8A8EAB             ; $8A8EA1
    lda.w $0B00,y               ; $8A8EA3
    ora.b $2C                   ; $8A8EA6
    sta.w $0B00,y               ; $8A8EA8
CODE_8A8EAB:
    lda.w $0500,x               ; $8A8EAB
    bmi CODE_8A8EB8             ; $8A8EAE
    lda.w $0B00,y               ; $8A8EB0
    ora.b $2E                   ; $8A8EB3
    sta.w $0B00,y               ; $8A8EB5
CODE_8A8EB8:
    ldy.b $2A                   ; $8A8EB8
    lda.w $0800,x               ; $8A8EBA
    sta.w $0902,y               ; $8A8EBD
    sep.b #$20                  ; $8A8EC0
    lda.b $32                   ; $8A8EC2
    sta.w $0900,y               ; $8A8EC4
    lda.b $26                   ; $8A8EC7
    sta.w $0901,y               ; $8A8EC9
    rep.b #$20                  ; $8A8ECC
    iny                         ; $8A8ECE
    iny                         ; $8A8ECF
    iny                         ; $8A8ED0
    iny                         ; $8A8ED1
    sty.b $2A                   ; $8A8ED2
    asl.b $2E                   ; $8A8ED4
    rol.b $2E                   ; $8A8ED6
    asl.b $2C                   ; $8A8ED8
    rol.b $2C                   ; $8A8EDA
    bcc CODE_8A8EEC             ; $8A8EDC
    lda.w #$0001                ; $8A8EDE
    sta.b $2C                   ; $8A8EE1
    lda.w #$0002                ; $8A8EE3
    sta.b $2E                   ; $8A8EE6
    inc.b $28                   ; $8A8EE8
    inc.b $28                   ; $8A8EEA
CODE_8A8EEC:
    jmp.w $8E63                 ; $8A8EEC
func_8A8EEF:
    rep.b #$20                  ; $8A8EEF
    stz.b $30                   ; $8A8EF1
    ldx.w #$00FE                ; $8A8EF3
CODE_8A8EF6:
    stz.w $0500,x               ; $8A8EF6
    dex                         ; $8A8EF9
    dex                         ; $8A8EFA
    bpl CODE_8A8EF6             ; $8A8EFB
    jsl.l clear_sprites         ; $8A8EFD
    rtl                         ; $8A8F01
clear_sprites:
    rep.b #$30                  ; $8A8F02
    ldx.w #$0080                ; $8A8F04
    ldy.w #$0000                ; $8A8F07
    lda.w #$E000                ; $8A8F0A
CODE_8A8F0D:
    sta.w $0900,y               ; $8A8F0D
    iny                         ; $8A8F10
    iny                         ; $8A8F11
    iny                         ; $8A8F12
    iny                         ; $8A8F13
    dex                         ; $8A8F14
    bne CODE_8A8F0D             ; $8A8F15
    rtl                         ; $8A8F17
hdma_gradients_init:
    sep.b #$20                  ; $8A8F18
    rep.b #$10                  ; $8A8F1A
    lda.b #$FF                  ; $8A8F1C
    sta.l $7E2C00               ; $8A8F1E
    sta.l $7E2C03               ; $8A8F22
    lda.b #$00                  ; $8A8F26
    sta.l $7E2C06               ; $8A8F28
    rep.b #$20                  ; $8A8F2C
    lda.w #$2800                ; $8A8F2E
    sta.l $7E2C01               ; $8A8F31
    clc                         ; $8A8F35
    adc.w #$0200                ; $8A8F36
    sta.l $7E2C04               ; $8A8F39
    lda.w #$2000                ; $8A8F3D
    sta.b $68                   ; $8A8F40
    lda.w #$007E                ; $8A8F42
    sta.b $6A                   ; $8A8F45
    lda.w #$0000                ; $8A8F47
    sta.b $6C                   ; $8A8F4A
    lda.w #$0000                ; $8A8F4C
    sta.b $3A                   ; $8A8F4F
    lda.w #$0000                ; $8A8F51
    sta.b $3C                   ; $8A8F54
    lda.w #$0000                ; $8A8F56
    sta.b $34                   ; $8A8F59
    sta.b $36                   ; $8A8F5B
    sta.b $38                   ; $8A8F5D
    jsl.l hdma_gradient_build   ; $8A8F5F
    lda.w $1536                 ; $8A8F63
    asl a                       ; $8A8F66
    asl a                       ; $8A8F67
    asl a                       ; $8A8F68
    tax                         ; $8A8F69
    lda.w #$0000                ; $8A8F6A
    sta.b $34                   ; $8A8F6D
    lda.w #$0000                ; $8A8F6F
    sta.b $36                   ; $8A8F72
    lda.l $8A934B,x             ; $8A8F74
    sta.b $38                   ; $8A8F78
    jsl.l hdma_gradient_build   ; $8A8F7A
    lda.w #$0001                ; $8A8F7E
    sta.b $3A                   ; $8A8F81
    lda.w #$0001                ; $8A8F83
    sta.b $3C                   ; $8A8F86
    lda.w $1536                 ; $8A8F88
    asl a                       ; $8A8F8B
    asl a                       ; $8A8F8C
    asl a                       ; $8A8F8D
    tax                         ; $8A8F8E
    lda.l $8A934B,x             ; $8A8F8F
    sta.b $34                   ; $8A8F93
    lda.l $8A934D,x             ; $8A8F95
    sta.b $36                   ; $8A8F99
    lda.b $72                   ; $8A8F9B
    sta.b $38                   ; $8A8F9D
    jsl.l hdma_gradient_build   ; $8A8F9F
    rtl                         ; $8A8FA3
hdma_gradient_build:
    rep.b #$30                  ; $8A8FA4
    lda.b $34                   ; $8A8FA6
    and.w #$001F                ; $8A8FA8
    xba                         ; $8A8FAB
    sta.w $1516                 ; $8A8FAC
    sta.w $1518                 ; $8A8FAF
    lda.b $36                   ; $8A8FB2
    and.w #$001F                ; $8A8FB4
    xba                         ; $8A8FB7
    sta.w $151A                 ; $8A8FB8
    lda.b $38                   ; $8A8FBB
    and.w #$001F                ; $8A8FBD
    xba                         ; $8A8FC0
    sta.w $151C                 ; $8A8FC1
    lda.w #$0000                ; $8A8FC4
    sta.w $151E                 ; $8A8FC7
    sec                         ; $8A8FCA
    lda.w $151C                 ; $8A8FCB
    sbc.w $151A                 ; $8A8FCE
    bpl CODE_8A8FD6             ; $8A8FD1
    ora.w #$00FF                ; $8A8FD3
CODE_8A8FD6:
    xba                         ; $8A8FD6
    asl a                       ; $8A8FD7
    sta.w $151C                 ; $8A8FD8
    sec                         ; $8A8FDB
    lda.w $151A                 ; $8A8FDC
    sbc.w $1518                 ; $8A8FDF
    bpl CODE_8A8FE7             ; $8A8FE2
    ora.w #$00FF                ; $8A8FE4
CODE_8A8FE7:
    xba                         ; $8A8FE7
    asl a                       ; $8A8FE8
    sta.w $151A                 ; $8A8FE9
    lda.b $34                   ; $8A8FEC
    and.w #$03E0                ; $8A8FEE
    asl a                       ; $8A8FF1
    asl a                       ; $8A8FF2
    asl a                       ; $8A8FF3
    sta.w $1520                 ; $8A8FF4
    sta.w $1522                 ; $8A8FF7
    lda.b $36                   ; $8A8FFA
    and.w #$03E0                ; $8A8FFC
    asl a                       ; $8A8FFF
    asl a                       ; $8A9000
    asl a                       ; $8A9001
    sta.w $1524                 ; $8A9002
    lda.b $38                   ; $8A9005
    and.w #$03E0                ; $8A9007
    asl a                       ; $8A900A
    asl a                       ; $8A900B
    asl a                       ; $8A900C
    sta.w $1526                 ; $8A900D
    lda.w #$0000                ; $8A9010
    sta.w $1528                 ; $8A9013
    sec                         ; $8A9016
    lda.w $1526                 ; $8A9017
    sbc.w $1524                 ; $8A901A
    bpl CODE_8A9022             ; $8A901D
    ora.w #$00FF                ; $8A901F
CODE_8A9022:
    xba                         ; $8A9022
    asl a                       ; $8A9023
    sta.w $1526                 ; $8A9024
    sec                         ; $8A9027
    lda.w $1524                 ; $8A9028
    sbc.w $1522                 ; $8A902B
    bpl CODE_8A9033             ; $8A902E
    ora.w #$00FF                ; $8A9030
CODE_8A9033:
    xba                         ; $8A9033
    asl a                       ; $8A9034
    sta.w $1524                 ; $8A9035
    lda.b $34                   ; $8A9038
    and.w #$7C00                ; $8A903A
    lsr a                       ; $8A903D
    lsr a                       ; $8A903E
    sta.w $152A                 ; $8A903F
    sta.w $152C                 ; $8A9042
    lda.b $36                   ; $8A9045
    and.w #$7C00                ; $8A9047
    lsr a                       ; $8A904A
    lsr a                       ; $8A904B
    sta.w $152E                 ; $8A904C
    lda.b $38                   ; $8A904F
    and.w #$7C00                ; $8A9051
    lsr a                       ; $8A9054
    lsr a                       ; $8A9055
    sta.w $1530                 ; $8A9056
    lda.w #$0000                ; $8A9059
    sta.w $1532                 ; $8A905C
    sec                         ; $8A905F
    lda.w $1530                 ; $8A9060
    sbc.w $152E                 ; $8A9063
    bpl CODE_8A906B             ; $8A9066
    ora.w #$00FF                ; $8A9068
CODE_8A906B:
    xba                         ; $8A906B
    asl a                       ; $8A906C
    sta.w $1530                 ; $8A906D
    sec                         ; $8A9070
    lda.w $152E                 ; $8A9071
    sbc.w $152C                 ; $8A9074
    bpl CODE_8A907C             ; $8A9077
    ora.w #$00FF                ; $8A9079
CODE_8A907C:
    xba                         ; $8A907C
    asl a                       ; $8A907D
    sta.w $152E                 ; $8A907E
    lda.w #$0080                ; $8A9081
    sta.b $70                   ; $8A9084
CODE_8A9086:
    lda.w $1518                 ; $8A9086
    lsr.w $151E                 ; $8A9089
    bcc CODE_8A9091             ; $8A908C
    lda.w $1516                 ; $8A908E
CODE_8A9091:
    and.w #$1F00                ; $8A9091
    xba                         ; $8A9094
    sta.b $3E                   ; $8A9095
    lda.w $1522                 ; $8A9097
    lsr.w $1528                 ; $8A909A
    bcc CODE_8A90A2             ; $8A909D
    lda.w $1520                 ; $8A909F
CODE_8A90A2:
    and.w #$1F00                ; $8A90A2
    lsr a                       ; $8A90A5
    lsr a                       ; $8A90A6
    lsr a                       ; $8A90A7
    ora.b $3E                   ; $8A90A8
    sta.b $3E                   ; $8A90AA
    lda.w $152C                 ; $8A90AC
    lsr.w $1532                 ; $8A90AF
    bcc CODE_8A90B7             ; $8A90B2
    lda.w $152A                 ; $8A90B4
CODE_8A90B7:
    and.w #$1F00                ; $8A90B7
    asl a                       ; $8A90BA
    asl a                       ; $8A90BB
    ora.b $3E                   ; $8A90BC
    sta.b $3E                   ; $8A90BE
    ldy.b $6C                   ; $8A90C0
    lda.w #$0000                ; $8A90C2
    sta.b [$68],y               ; $8A90C5
    iny                         ; $8A90C7
    iny                         ; $8A90C8
    lda.b $3E                   ; $8A90C9
    sta.b [$68],y               ; $8A90CB
    iny                         ; $8A90CD
    iny                         ; $8A90CE
    sty.b $6C                   ; $8A90CF
    ldx.w $1518                 ; $8A90D1
    clc                         ; $8A90D4
    txa                         ; $8A90D5
    adc.w $151A                 ; $8A90D6
    tay                         ; $8A90D9
    and.w #$FF00                ; $8A90DA
    sta.w $1518                 ; $8A90DD
    txa                         ; $8A90E0
    and.w #$FF00                ; $8A90E1
    cmp.w $1518                 ; $8A90E4
    beq CODE_8A90F1             ; $8A90E7
    lda.b $3A                   ; $8A90E9
    sta.w $151E                 ; $8A90EB
    stx.w $1516                 ; $8A90EE
CODE_8A90F1:
    sty.w $1518                 ; $8A90F1
    ldx.w $1522                 ; $8A90F4
    clc                         ; $8A90F7
    txa                         ; $8A90F8
    adc.w $1524                 ; $8A90F9
    tay                         ; $8A90FC
    and.w #$FF00                ; $8A90FD
    sta.w $1522                 ; $8A9100
    txa                         ; $8A9103
    and.w #$FF00                ; $8A9104
    cmp.w $1522                 ; $8A9107
    beq CODE_8A9114             ; $8A910A
    lda.b $3A                   ; $8A910C
    sta.w $1528                 ; $8A910E
    stx.w $1520                 ; $8A9111
CODE_8A9114:
    sty.w $1522                 ; $8A9114
    ldx.w $152C                 ; $8A9117
    clc                         ; $8A911A
    txa                         ; $8A911B
    adc.w $152E                 ; $8A911C
    tay                         ; $8A911F
    and.w #$FF00                ; $8A9120
    sta.w $152C                 ; $8A9123
    txa                         ; $8A9126
    and.w #$FF00                ; $8A9127
    cmp.w $152C                 ; $8A912A
    beq CODE_8A9137             ; $8A912D
    lda.b $3A                   ; $8A912F
    sta.w $1532                 ; $8A9131
    stx.w $152A                 ; $8A9134
CODE_8A9137:
    sty.w $152C                 ; $8A9137
    dec.b $70                   ; $8A913A
    beq CODE_8A9141             ; $8A913C
    brl CODE_8A9086             ; $8A913E
CODE_8A9141:
    lda.w #$0080                ; $8A9141
    sta.b $70                   ; $8A9144
CODE_8A9146:
    lda.w $1518                 ; $8A9146
    lsr.w $151E                 ; $8A9149
    bcc CODE_8A9151             ; $8A914C
    lda.w $1516                 ; $8A914E
CODE_8A9151:
    and.w #$1F00                ; $8A9151
    xba                         ; $8A9154
    sta.b $3E                   ; $8A9155
    lda.w $1522                 ; $8A9157
    lsr.w $1528                 ; $8A915A
    bcc CODE_8A9162             ; $8A915D
    lda.w $1520                 ; $8A915F
CODE_8A9162:
    and.w #$1F00                ; $8A9162
    lsr a                       ; $8A9165
    lsr a                       ; $8A9166
    lsr a                       ; $8A9167
    ora.b $3E                   ; $8A9168
    sta.b $3E                   ; $8A916A
    lda.w $152C                 ; $8A916C
    lsr.w $1532                 ; $8A916F
    bcc CODE_8A9177             ; $8A9172
    lda.w $152A                 ; $8A9174
CODE_8A9177:
    and.w #$1F00                ; $8A9177
    asl a                       ; $8A917A
    asl a                       ; $8A917B
    ora.b $3E                   ; $8A917C
    sta.b $3E                   ; $8A917E
    ldy.b $6C                   ; $8A9180
    lda.w #$0000                ; $8A9182
    sta.b [$68],y               ; $8A9185
    iny                         ; $8A9187
    iny                         ; $8A9188
    lda.b $3E                   ; $8A9189
    sta.b [$68],y               ; $8A918B
    iny                         ; $8A918D
    iny                         ; $8A918E
    sty.b $6C                   ; $8A918F
    ldx.w $1518                 ; $8A9191
    clc                         ; $8A9194
    txa                         ; $8A9195
    adc.w $151C                 ; $8A9196
    tay                         ; $8A9199
    and.w #$FF00                ; $8A919A
    sta.w $1518                 ; $8A919D
    txa                         ; $8A91A0
    and.w #$FF00                ; $8A91A1
    cmp.w $1518                 ; $8A91A4
    beq CODE_8A91B1             ; $8A91A7
    lda.b $3C                   ; $8A91A9
    sta.w $151E                 ; $8A91AB
    stx.w $1516                 ; $8A91AE
CODE_8A91B1:
    sty.w $1518                 ; $8A91B1
    ldx.w $1522                 ; $8A91B4
    clc                         ; $8A91B7
    txa                         ; $8A91B8
    adc.w $1526                 ; $8A91B9
    tay                         ; $8A91BC
    and.w #$FF00                ; $8A91BD
    sta.w $1522                 ; $8A91C0
    txa                         ; $8A91C3
    and.w #$FF00                ; $8A91C4
    cmp.w $1522                 ; $8A91C7
    beq CODE_8A91D4             ; $8A91CA
    lda.b $3C                   ; $8A91CC
    sta.w $1528                 ; $8A91CE
    stx.w $1520                 ; $8A91D1
CODE_8A91D4:
    sty.w $1522                 ; $8A91D4
    ldx.w $152C                 ; $8A91D7
    clc                         ; $8A91DA
    txa                         ; $8A91DB
    adc.w $1530                 ; $8A91DC
    tay                         ; $8A91DF
    and.w #$FF00                ; $8A91E0
    sta.w $152C                 ; $8A91E3
    txa                         ; $8A91E6
    and.w #$FF00                ; $8A91E7
    cmp.w $152C                 ; $8A91EA
    beq CODE_8A91F7             ; $8A91ED
    lda.b $3C                   ; $8A91EF
    sta.w $1532                 ; $8A91F1
    stx.w $152A                 ; $8A91F4
CODE_8A91F7:
    sty.w $152C                 ; $8A91F7
    dec.b $70                   ; $8A91FA
    beq CODE_8A9201             ; $8A91FC
    brl CODE_8A9146             ; $8A91FE
CODE_8A9201:
    rtl                         ; $8A9201
func_8A9202:
    lda.b $7C                   ; $8A9202
    pha                         ; $8A9204
    lda.b $7A                   ; $8A9205
    pha                         ; $8A9207
    pha                         ; $8A9208
    clc                         ; $8A9209
    sta.w $06C0,x               ; $8A920A
    adc.w #$0010                ; $8A920D
    sta.w $06C2,x               ; $8A9210
    adc.w #$0010                ; $8A9213
    sta.w $06C4,x               ; $8A9216
    adc.w #$0010                ; $8A9219
    sta.w $06C6,x               ; $8A921C
    adc.w #$0010                ; $8A921F
    sta.w $06C8,x               ; $8A9222
    pla                         ; $8A9225
    sta.w $06CA,x               ; $8A9226
    adc.w #$0010                ; $8A9229
    sta.w $06CC,x               ; $8A922C
    adc.w #$0010                ; $8A922F
    sta.w $06CE,x               ; $8A9232
    adc.w #$0010                ; $8A9235
    sta.w $06D0,x               ; $8A9238
    adc.w #$0010                ; $8A923B
    sta.w $06D2,x               ; $8A923E
    clc                         ; $8A9241
    lda.b $7C                   ; $8A9242
    sta.w $07C0,x               ; $8A9244
    sta.w $07C2,x               ; $8A9247
    sta.w $07C4,x               ; $8A924A
    sta.w $07C6,x               ; $8A924D
    sta.w $07C8,x               ; $8A9250
    adc.w #$0010                ; $8A9253
    sta.w $07CA,x               ; $8A9256
    sta.w $07CC,x               ; $8A9259
    sta.w $07CE,x               ; $8A925C
    sta.w $07D0,x               ; $8A925F
    sta.w $07D2,x               ; $8A9262
    lda.w #$0001                ; $8A9265
    sta.w $05C0,x               ; $8A9268
    sta.w $05C2,x               ; $8A926B
    sta.w $05C4,x               ; $8A926E
    sta.w $05C6,x               ; $8A9271
    sta.w $05C8,x               ; $8A9274
    sta.w $05CA,x               ; $8A9277
    sta.w $05CC,x               ; $8A927A
    sta.w $05CE,x               ; $8A927D
    sta.w $05D0,x               ; $8A9280
    sta.w $05D2,x               ; $8A9283
    lda.w #$04AA                ; $8A9286
    sta.w $08C0,x               ; $8A9289
    lda.w #$04AC                ; $8A928C
    sta.w $08C2,x               ; $8A928F
    lda.w #$04AE                ; $8A9292
    sta.w $08C4,x               ; $8A9295
    lda.w #$04C0                ; $8A9298
    sta.w $08C6,x               ; $8A929B
    lda.w #$04C2                ; $8A929E
    sta.w $08C8,x               ; $8A92A1
    lda.w #$04C4                ; $8A92A4
    sta.w $08CA,x               ; $8A92A7
    lda.w #$04C6                ; $8A92AA
    sta.w $08CC,x               ; $8A92AD
    lda.w #$04C8                ; $8A92B0
    sta.w $08CE,x               ; $8A92B3
    lda.w #$04CA                ; $8A92B6
    sta.w $08D0,x               ; $8A92B9
    lda.w #$04CC                ; $8A92BC
    sta.w $08D2,x               ; $8A92BF
    txa                         ; $8A92C2
    clc                         ; $8A92C3
    adc.w #$0014                ; $8A92C4
    tax                         ; $8A92C7
    pla                         ; $8A92C8
    sta.b $7A                   ; $8A92C9
    pla                         ; $8A92CB
    sta.b $7C                   ; $8A92CC
    rts                         ; $8A92CE
func_8A92CF:
    lda.b $7C                   ; $8A92CF
    pha                         ; $8A92D1
    lda.b $7A                   ; $8A92D2
    pha                         ; $8A92D4
    clc                         ; $8A92D5
    sta.w $06C0,x               ; $8A92D6
    adc.w #$0010                ; $8A92D9
    sta.w $06C2,x               ; $8A92DC
    lda.b $7C                   ; $8A92DF
    sta.w $07C0,x               ; $8A92E1
    sta.w $07C2,x               ; $8A92E4
    lda.w #$0001                ; $8A92E7
    sta.w $05C0,x               ; $8A92EA
    sta.w $05C2,x               ; $8A92ED
    lda.w #$04A0                ; $8A92F0
    sta.w $08C0,x               ; $8A92F3
    lda.w #$04A2                ; $8A92F6
    sta.w $08C2,x               ; $8A92F9
    inx                         ; $8A92FC
    inx                         ; $8A92FD
    inx                         ; $8A92FE
    inx                         ; $8A92FF
    pla                         ; $8A9300
    sta.b $7A                   ; $8A9301
    pla                         ; $8A9303
    sta.b $7C                   ; $8A9304
    rts                         ; $8A9306
    incbin !baserom:$51307..$514C3
results_screen:
    lda.w #$00FF                ; $8A94C3
    trb.w $1B38                 ; $8A94C6
    jsl.l text_map_clear        ; $8A94C9
    jsl.l variant_offsets_add   ; $8A94CD
    jsl.l results_text          ; $8A94D1
    jsl.l clear_vram            ; $8A94D5
    stz.w $0196                 ; $8A94D9
    stz.w $0198                 ; $8A94DC
    stz.w $019A                 ; $8A94DF
    ldx.w #$9D31                ; $8A94E2
    lda.w #$008A                ; $8A94E5
    jsl.l video_load_screen_long; $8A94E8
    lda.w #$8934                ; $8A94EC
    sta.w $0008                 ; $8A94EF
    ldx.w #$8601                ; $8A94F2
    ldy.w #$1000                ; $8A94F5
    lda.w #$0088                ; $8A94F8
    jsl.l decompress_to_vram_long; $8A94FB
    lda.w #$8660                ; $8A94FF
    ldx.w #$F40B                ; $8A9502
    ldy.w #$0040                ; $8A9505
    jsl.l upload_cgram_now      ; $8A9508
    lda.w #$8A10                ; $8A950C
    ldx.w #$9D2D                ; $8A950F
    ldy.w #$0020                ; $8A9512
    jsl.l upload_cgram_now      ; $8A9515
    ldx.w #$AADF                ; $8A9519
    ldy.w #$7400                ; $8A951C
    lda.w #$0091                ; $8A951F
    jsl.l decompress_to_vram_long; $8A9522
    rep.b #$20                  ; $8A9526
    ldx.w #$0000                ; $8A9528
    ldy.w #$3800                ; $8A952B
    lda.w #$0000                ; $8A952E
CODE_8A9531:
    sta.l $7F0800,x             ; $8A9531
    inx                         ; $8A9535
    inx                         ; $8A9536
    dey                         ; $8A9537
    bne CODE_8A9531             ; $8A9538
    ldx.w #$0D0B                ; $8A953A
    ldy.w #$4000                ; $8A953D
    pea.w $817F                 ; $8A9540
    plb                         ; $8A9543
    plb                         ; $8A9544
    lda.w #$5000                ; $8A9545
    sta.w $2116                 ; $8A9548
    ldx.w #$0010                ; $8A954B
CODE_8A954E:
    stz.w $2118                 ; $8A954E
    dex                         ; $8A9551
    bne CODE_8A954E             ; $8A9552
    jsl.l oam_clear_long        ; $8A9554
    phb                         ; $8A9558
    lda.w #$8C80                ; $8A9559
    ldx.w #$95C3                ; $8A955C
    ldy.w #$0020                ; $8A955F
    jsl.l upload_cgram_now      ; $8A9562
    lda.w #$8600                ; $8A9566
    ldx.w #$F44B                ; $8A9569
    ldy.w #$0020                ; $8A956C
    jsl.l upload_cgram_now      ; $8A956F
    ldy.w #$0800                ; $8A9573
    ldx.w #$F861                ; $8A9576
    lda.w #$0087                ; $8A9579
    jsl.l decompress_to_vram_long; $8A957C
    plb                         ; $8A9580
    jsr.w clear_tilemap_7f      ; $8A9581
    inc.b $6A                   ; $8A9584
    jsr.w stars_scatter_8a      ; $8A9586
    lda.w #$0000                ; $8A9589
    ldx.w #$0040                ; $8A958C
    jsr.w stars_draw_8a         ; $8A958F
    jsl.l oam_finish_0b00       ; $8A9592
    sep.b #$20                  ; $8A9596
    lda.b #$08                  ; $8A9598
    sta.w $2110                 ; $8A959A
    stz.w $2110                 ; $8A959D
    rep.b #$20                  ; $8A95A0
    lda.w #$27FF                ; $8A95A2
    sta.b $7A                   ; $8A95A5
    stz.b $7C                   ; $8A95A7
    stz.b $80                   ; $8A95A9
    stz.b $82                   ; $8A95AB
    lda.w #$4000                ; $8A95AD
    sta.b $86                   ; $8A95B0
    pea.w $8A8A                 ; $8A95B2
    plb                         ; $8A95B5
    plb                         ; $8A95B6
    jsr.w hdma_coldata_setup    ; $8A95B7
    stz.w $000E                 ; $8A95BA
    stz.w $0012                 ; $8A95BD
    stz.b $40                   ; $8A95C0
    lda.w #$0100                ; $8A95C2
    sta.b $7C                   ; $8A95C5
    stz.b $86                   ; $8A95C7
    stz.b $3A                   ; $8A95C9
    lda.w #$FC14                ; $8A95CB
    sta.w $0008                 ; $8A95CE
    lda.w #$FFFF                ; $8A95D1
    sta.b $90                   ; $8A95D4
    lda.w #$0001                ; $8A95D6
    sta.b $2C                   ; $8A95D9
    lda.w #$0002                ; $8A95DB
    sta.b $3A                   ; $8A95DE
    lda.w #$0007                ; $8A95E0
    sta.b $3C                   ; $8A95E3
    jsr.w text_buffer_screen    ; $8A95E5
    jsr.w upload_7f_to_3000     ; $8A95E8
results_screen_show:
    lda.w #$0001                ; $8A95EB
    jsl.l wait_frames_8a9880    ; $8A95EE
    lda.w #$0004                ; $8A95F2
    sta.w $420C                 ; $8A95F5
    lda.w #$0F00                ; $8A95F8
    sta.w $0198                 ; $8A95FB
    lda.w #$0080                ; $8A95FE
    sta.w $0196                 ; $8A9601
    stz.b $6E                   ; $8A9604
    lda.w #$0001                ; $8A9606
    jsl.l wait_frames_8a9880    ; $8A9609
    lda.b $30                   ; $8A960D
    asl a                       ; $8A960F
    tax                         ; $8A9610
    lda.w #$1801                ; $8A9611
    sta.w $4300                 ; $8A9614
    lda.w #$0800                ; $8A9617
    sta.w $4302                 ; $8A961A
    lda.w #$0800                ; $8A961D
    sta.w $4305                 ; $8A9620
    lda.w #$4000                ; $8A9623
    sta.w $2116                 ; $8A9626
    sep.b #$20                  ; $8A9629
    lda.b #$7F                  ; $8A962B
    sta.w $4304                 ; $8A962D
    lda.b #$01                  ; $8A9630
    sta.w $420B                 ; $8A9632
    rep.b #$20                  ; $8A9635
    jsr.w func_8A96C2           ; $8A9637
    lda.b $86                   ; $8A963A
    and.w #$FF00                ; $8A963C
    xba                         ; $8A963F
    tax                         ; $8A9640
    lda.b $7C                   ; $8A9641
    jsr.w stars_draw_8a         ; $8A9643
    jsl.l oam_finish_0b00       ; $8A9646
    lda.w #$0002                ; $8A964A
    tsb.w $0026                 ; $8A964D
    bit.w $1B37                 ; $8A9650
    bpl CODE_8A965E             ; $8A9653
    incbin !baserom:$51655..$5165E
CODE_8A965E:
    jsr.w fade_out_on_confirm   ; $8A965E
    lda.w #$0001                ; $8A9661
    trb.b $F8                   ; $8A9664
    beq CODE_8A9668             ; $8A9666
CODE_8A9668:
    lda.l $81019A               ; $8A9668
    beq CODE_8A9674             ; $8A966C
    jsr.w hdma_table_0d00_settle; $8A966E
    jmp.w $9606                 ; $8A9671
CODE_8A9674:
    rtl                         ; $8A9674
fade_out_on_confirm:
    lda.w #$C0C0                ; $8A9675
    trb.w $000E                 ; $8A9678
    bne CODE_8A9686             ; $8A967B
    lda.w #$C0C0                ; $8A967D
    trb.w $0012                 ; $8A9680
    bne CODE_8A9686             ; $8A9683
    rts                         ; $8A9685
CODE_8A9686:
    stz.b $98                   ; $8A9686
    lda.w #$FF80                ; $8A9688
    sta.b $96                   ; $8A968B
    rts                         ; $8A968D
    lda.w #$0002                ; $8A968E
    tsb.w $0026                 ; $8A9691
    rep.b #$30                  ; $8A9694
    phb                         ; $8A9696
    pea.w $8A8A                 ; $8A9697
    plb                         ; $8A969A
    plb                         ; $8A969B
    lda.w #$0100                ; $8A969C
    tcd                         ; $8A969F
    jsl.l func_8A987F           ; $8A96A0
    plb                         ; $8A96A4
    lda.w #$0000                ; $8A96A5
    tcd                         ; $8A96A8
    jml.l $80FC18               ; $8A96A9
    incbin !baserom:$516AD..$516C2
func_8A96C2:
    inc.b $40                   ; $8A96C2
    lda.b $40                   ; $8A96C4
    clc                         ; $8A96C6
    rts                         ; $8A96C7
    incbin !baserom:$516C8..$5171B
stars_scatter_8a:
    ldx.w #$0000                ; $8A971B
    lda.w #$2000                ; $8A971E
    sta.b $14                   ; $8A9721
CODE_8A9723:
    jsr.w rng_next_8a           ; $8A9723
    sep.b #$20                  ; $8A9726
    pha                         ; $8A9728
    lda.b $14                   ; $8A9729
    sta.w $211B                 ; $8A972B
    lda.b $15                   ; $8A972E
    sta.w $211B                 ; $8A9730
    pla                         ; $8A9733
    sta.w $211C                 ; $8A9734
    rep.b #$20                  ; $8A9737
    lda.w $2135                 ; $8A9739
    sta.w $0F00,x               ; $8A973C
    jsr.w rng_next_8a           ; $8A973F
    xba                         ; $8A9742
    sep.b #$20                  ; $8A9743
    pha                         ; $8A9745
    lda.b $14                   ; $8A9746
    sta.w $211B                 ; $8A9748
    lda.b $15                   ; $8A974B
    sta.w $211B                 ; $8A974D
    pla                         ; $8A9750
    sta.w $211C                 ; $8A9751
    rep.b #$20                  ; $8A9754
    lda.w $2135                 ; $8A9756
    sta.w $0F02,x               ; $8A9759
    lda.b $14                   ; $8A975C
    sta.w $0F04,x               ; $8A975E
    clc                         ; $8A9761
    adc.w #$00BE                ; $8A9762
    sta.b $14                   ; $8A9765
    txa                         ; $8A9767
    clc                         ; $8A9768
    adc.w #$0006                ; $8A9769
    tax                         ; $8A976C
    cpx.w #$0300                ; $8A976D
    bne CODE_8A9723             ; $8A9770
    rts                         ; $8A9772
stars_draw_8a:
    stx.b $18                   ; $8A9773
    sta.b $16                   ; $8A9775
    ldy.w #$0000                ; $8A9777
    ldx.b $74                   ; $8A977A
    sep.b #$20                  ; $8A977C
    lda.w $0F05,y               ; $8A977E
    sta.w $4202                 ; $8A9781
    lda.b $18                   ; $8A9784
    sta.w $4203                 ; $8A9786
    rep.b #$20                  ; $8A9789
    xba                         ; $8A978B
    nop                         ; $8A978C
    lda.w $4216                 ; $8A978D
    sta.b $19                   ; $8A9790
    clc                         ; $8A9792
    adc.w $0F00,y               ; $8A9793
    bpl CODE_8A97BD             ; $8A9796
    eor.w #$FFFF                ; $8A9798
    inc a                       ; $8A979B
    sta.w $4204                 ; $8A979C
    sep.b #$20                  ; $8A979F
    lda.w $0F05,y               ; $8A97A1
    sta.w $4206                 ; $8A97A4
    rep.b #$20                  ; $8A97A7
    xba                         ; $8A97A9
    xba                         ; $8A97AA
    nop                         ; $8A97AB
    lda.w #$0080                ; $8A97AC
    sec                         ; $8A97AF
    sbc.w $4214                 ; $8A97B0
    bit.w #$FF00                ; $8A97B3
    bne CODE_8A97BA             ; $8A97B6
    bra CODE_8A97D9             ; $8A97B8
CODE_8A97BA:
    jmp.w $9848                 ; $8A97BA
CODE_8A97BD:
    sta.w $4204                 ; $8A97BD
    sep.b #$20                  ; $8A97C0
    lda.w $0F05,y               ; $8A97C2
    sta.w $4206                 ; $8A97C5
    rep.b #$20                  ; $8A97C8
    xba                         ; $8A97CA
    xba                         ; $8A97CB
    nop                         ; $8A97CC
    lda.w #$0080                ; $8A97CD
    clc                         ; $8A97D0
    adc.w $4214                 ; $8A97D1
    bit.w #$FF00                ; $8A97D4
    bne CODE_8A9848             ; $8A97D7
CODE_8A97D9:
    sta.b $14                   ; $8A97D9
    lda.b $19                   ; $8A97DB
    eor.w #$FFFF                ; $8A97DD
    sec                         ; $8A97E0
    adc.w $0F02,y               ; $8A97E1
    bmi CODE_8A97FF             ; $8A97E4
    sta.w $4204                 ; $8A97E6
    sep.b #$20                  ; $8A97E9
    lda.w $0F05,y               ; $8A97EB
    sta.w $4206                 ; $8A97EE
    rep.b #$20                  ; $8A97F1
    xba                         ; $8A97F3
    xba                         ; $8A97F4
    nop                         ; $8A97F5
    lda.w #$005C                ; $8A97F6
    clc                         ; $8A97F9
    adc.w $4214                 ; $8A97FA
    bra CODE_8A981A             ; $8A97FD
CODE_8A97FF:
    eor.w #$FFFF                ; $8A97FF
    inc a                       ; $8A9802
    sta.w $4204                 ; $8A9803
    sep.b #$20                  ; $8A9806
    lda.w $0F05,y               ; $8A9808
    sta.w $4206                 ; $8A980B
    rep.b #$20                  ; $8A980E
    xba                         ; $8A9810
    xba                         ; $8A9811
    nop                         ; $8A9812
    lda.w #$005C                ; $8A9813
    sec                         ; $8A9816
    sbc.w $4214                 ; $8A9817
CODE_8A981A:
    cmp.w #$00E0                ; $8A981A
    bcs CODE_8A9848             ; $8A981D
    and.w #$00FF                ; $8A981F
    xba                         ; $8A9822
    ora.b $14                   ; $8A9823
    sta.w $0900,x               ; $8A9825
    lda.w $0F05,y               ; $8A9828
    and.w #$007C                ; $8A982B
    phx                         ; $8A982E
    tax                         ; $8A982F
    lda.l $86F46B,x             ; $8A9830
    ora.b $80                   ; $8A9834
    plx                         ; $8A9836
    sta.w $0902,x               ; $8A9837
    stz.w $0B01,x               ; $8A983A
    lda.w $0F04,y               ; $8A983D
    sec                         ; $8A9840
    sbc.b $16                   ; $8A9841
    cmp.w #$0100                ; $8A9843
    bcs CODE_8A984E             ; $8A9846
CODE_8A9848:
    jsr.w star_spawn_8a         ; $8A9848
CODE_8A984B:
    jmp.w $977C                 ; $8A984B
CODE_8A984E:
    sta.w $0F04,y               ; $8A984E
    inx                         ; $8A9851
    inx                         ; $8A9852
    inx                         ; $8A9853
    inx                         ; $8A9854
    tya                         ; $8A9855
    clc                         ; $8A9856
    adc.w #$0006                ; $8A9857
    tay                         ; $8A985A
    cpy.w #$0300                ; $8A985B
    bne CODE_8A984B             ; $8A985E
    stx.b $74                   ; $8A9860
    rts                         ; $8A9862
star_spawn_8a:
    jsr.w rng_next_8a           ; $8A9863
    cmp.w #$8000                ; $8A9866
    ror a                       ; $8A9869
    sta.w $0F00,y               ; $8A986A
    jsr.w rng_next_8a           ; $8A986D
    xba                         ; $8A9870
    cmp.w #$8000                ; $8A9871
    ror a                       ; $8A9874
    sta.w $0F02,y               ; $8A9875
    lda.w #$7F00                ; $8A9878
    sta.w $0F04,y               ; $8A987B
    rts                         ; $8A987E
func_8A987F:
    rtl                         ; $8A987F
wait_frames_8a9880:
    clc                         ; $8A9880
    adc.l $810024               ; $8A9881
CODE_8A9885:
    cmp.l $810024               ; $8A9885
    bne CODE_8A9885             ; $8A9889
    rtl                         ; $8A988B
rng_next_8a:
    lda.b $12                   ; $8A988C
    bmi CODE_8A98A3             ; $8A988E
    lda.b $10                   ; $8A9890
    asl a                       ; $8A9892
    eor.w #$2B45                ; $8A9893
    sta.b $10                   ; $8A9896
    lda.b $12                   ; $8A9898
    rol a                       ; $8A989A
    eor.w #$1D87                ; $8A989B
    sta.b $12                   ; $8A989E
    lda.b $10                   ; $8A98A0
    rts                         ; $8A98A2
CODE_8A98A3:
    asl.b $10                   ; $8A98A3
    rol.b $12                   ; $8A98A5
    lda.b $10                   ; $8A98A7
    rts                         ; $8A98A9
    incbin !baserom:$518AA..$518B4
variant_offsets_add:
    jsr.w variant_5a            ; $8A98B4
    ldx.w #$0000                ; $8A98B7
CODE_8A98BA:
    phx                         ; $8A98BA
    lda.l $810446,x             ; $8A98BB
    and.w #$00FF                ; $8A98BF
    asl a                       ; $8A98C2
    tay                         ; $8A98C3
    txa                         ; $8A98C4
    asl a                       ; $8A98C5
    tax                         ; $8A98C6
    lda.l $8103DC               ; $8A98C7
    and.w #$000F                ; $8A98CB
    cmp.w #$0002                ; $8A98CE
    bne CODE_8A98DF             ; $8A98D1
    clc                         ; $8A98D3 static
    lda.w $041C,y               ; $8A98D4 static
    adc.w $9DB5,x               ; $8A98D7 static
    sta.w $041C,y               ; $8A98DA static
    bra CODE_8A98E9             ; $8A98DD static
CODE_8A98DF:
    clc                         ; $8A98DF
    lda.w $041C,y               ; $8A98E0
    adc.w $9D8D,x               ; $8A98E3
    sta.w $041C,y               ; $8A98E6
CODE_8A98E9:
    plx                         ; $8A98E9
    inx                         ; $8A98EA
    dec.b $5A                   ; $8A98EB
    bne CODE_8A98BA             ; $8A98ED
    rtl                         ; $8A98EF
variant_5a:
    lda.l $8103DC               ; $8A98F0
    and.w #$000F                ; $8A98F4
    cmp.w #$0002                ; $8A98F7
    bne CODE_8A9902             ; $8A98FA
    lda.w #$0004                ; $8A98FC static
    sta.b $5A                   ; $8A98FF static
    rts                         ; $8A9901 static
CODE_8A9902:
    lda.w #$000A                ; $8A9902
    sta.b $5A                   ; $8A9905
    rts                         ; $8A9907
results_text:
    jsr.w results_text_build    ; $8A9908
    jsr.w func_8A9BE2           ; $8A990B
    stz.b $5E                   ; $8A990E
    jsr.w variant_66            ; $8A9910
CODE_8A9913:
    jsr.w func_8A9BB5           ; $8A9913
    bcs CODE_8A993E             ; $8A9916
    inc.b $5E                   ; $8A9918
    jsr.w byte_buffer_decimal2  ; $8A991A
    jsr.w byte_buffer_space     ; $8A991D
    jsr.w text_line_push        ; $8A9920
    lda.w #$0016                ; $8A9923
    jsr.w byte_buffer_pad       ; $8A9926
    jsr.w byte_buffer_space     ; $8A9929
    jsr.w byte_buffer_space     ; $8A992C
    jsr.w byte_buffer_space     ; $8A992F
    lda.l $81041C,x             ; $8A9932
    jsr.w byte_buffer_decimal   ; $8A9936
    jsr.w byte_buffer_end       ; $8A9939
    bra CODE_8A9913             ; $8A993C
CODE_8A993E:
    bit.w $1B37                 ; $8A993E
    bmi CODE_8A9946             ; $8A9941
    jmp.w $999F                 ; $8A9943
CODE_8A9946:
    jsr.w byte_buffer_end       ; $8A9946 static
    jsr.w byte_buffer_end       ; $8A9949 static
    sep.b #$20                  ; $8A994C static
    ldx.w #$0000                ; $8A994E static
CODE_8A9951:
    lda.w $9DBF,x               ; $8A9951 static
    beq CODE_8A995C             ; $8A9954 static
    jsr.w byte_buffer_push      ; $8A9956 static
    inx                         ; $8A9959 static
    bra CODE_8A9951             ; $8A995A static
CODE_8A995C:
    rep.b #$20                  ; $8A995C static
    jsr.w func_8A9BE2           ; $8A995E static
    stz.b $5E                   ; $8A9961 static
    lda.w #$0008                ; $8A9963 static
    sta.b $66                   ; $8A9966 static
    lda.w #$0000                ; $8A9968 static
    sta.l $7F7FBC               ; $8A996B static
    sta.l $7F7FBE               ; $8A996F static
    sta.l $7F7FC0               ; $8A9973 static
    sta.l $7F7FC2               ; $8A9977 static
    jsr.w func_8A9BB5           ; $8A997B static
    jsr.w byte_buffer_end       ; $8A997E static
    jsr.w byte_buffer_end       ; $8A9981 static
    jsr.w text_line_centered    ; $8A9984 static
    jsr.w byte_buffer_end       ; $8A9987 static
    jsr.w byte_buffer_end       ; $8A998A static
    sep.b #$20                  ; $8A998D static
    ldx.w #$0000                ; $8A998F static
CODE_8A9992:
    lda.w $9DD5,x               ; $8A9992 static
    beq CODE_8A999D             ; $8A9995 static
    jsr.w byte_buffer_push      ; $8A9997 static
    inx                         ; $8A999A static
    bra CODE_8A9992             ; $8A999B static
CODE_8A999D:
    rep.b #$20                  ; $8A999D static
    sep.b #$20                  ; $8A999F
    ldx.w #$0000                ; $8A99A1
CODE_8A99A4:
    lda.w $9DBD,x               ; $8A99A4
    cmp.b #$FF                  ; $8A99A7
    beq CODE_8A99B1             ; $8A99A9
    jsr.w byte_buffer_push      ; $8A99AB
    inx                         ; $8A99AE
    bra CODE_8A99A4             ; $8A99AF
CODE_8A99B1:
    rep.b #$20                  ; $8A99B1
    rtl                         ; $8A99B3
byte_buffer_pad:
    cmp.b $60                   ; $8A99B4
    beq CODE_8A99BD             ; $8A99B6
    jsr.w byte_buffer_space     ; $8A99B8
    bra byte_buffer_pad         ; $8A99BB
CODE_8A99BD:
    rts                         ; $8A99BD
variant_66:
    lda.l $8103DC               ; $8A99BE
    and.w #$000F                ; $8A99C2
    cmp.w #$0002                ; $8A99C5
    bne func_8A99D0             ; $8A99C8
    lda.w #$0008                ; $8A99CA static
    sta.b $66                   ; $8A99CD static
    rts                         ; $8A99CF static
func_8A99D0:
    lda.w #$0028                ; $8A99D0
    sta.b $66                   ; $8A99D3
    rts                         ; $8A99D5
byte_buffer_end:
    sep.b #$20                  ; $8A99D6
    lda.b #$80                  ; $8A99D8
    jsr.w byte_buffer_push      ; $8A99DA
    rep.b #$20                  ; $8A99DD
    stz.b $60                   ; $8A99DF
    rts                         ; $8A99E1
text_line_push:
    phx                         ; $8A99E2
    txa                         ; $8A99E3
    lsr a                       ; $8A99E4
    asl a                       ; $8A99E5
    asl a                       ; $8A99E6
    asl a                       ; $8A99E7
    asl a                       ; $8A99E8
    tax                         ; $8A99E9
    sep.b #$20                  ; $8A99EA
CODE_8A99EC:
    lda.l $7F7918,x             ; $8A99EC
    beq CODE_8A99F8             ; $8A99F0
    jsr.w byte_buffer_push      ; $8A99F2
    inx                         ; $8A99F5
    bra CODE_8A99EC             ; $8A99F6
CODE_8A99F8:
    rep.b #$20                  ; $8A99F8
    plx                         ; $8A99FA
    rts                         ; $8A99FB
text_line_centered:
    phx                         ; $8A99FC static
    txa                         ; $8A99FD static
    lsr a                       ; $8A99FE static
    asl a                       ; $8A99FF static
    asl a                       ; $8A9A00 static
    asl a                       ; $8A9A01 static
    asl a                       ; $8A9A02 static
    tax                         ; $8A9A03 static
    sep.b #$20                  ; $8A9A04 static
    phx                         ; $8A9A06 static
    ldy.w #$001C                ; $8A9A07 static
CODE_8A9A0A:
    lda.l $7F7918,x             ; $8A9A0A static
    beq CODE_8A9A14             ; $8A9A0E static
    dey                         ; $8A9A10 static
    inx                         ; $8A9A11 static
    bra CODE_8A9A0A             ; $8A9A12 static
CODE_8A9A14:
    plx                         ; $8A9A14 static
    rep.b #$20                  ; $8A9A15 static
    tya                         ; $8A9A17 static
    lsr a                       ; $8A9A18 static
CODE_8A9A19:
    pha                         ; $8A9A19 static
    jsr.w byte_buffer_space     ; $8A9A1A static
    pla                         ; $8A9A1D static
    dec a                       ; $8A9A1E static
    bne CODE_8A9A19             ; $8A9A1F static
    sep.b #$20                  ; $8A9A21 static
CODE_8A9A23:
    lda.l $7F7918,x             ; $8A9A23 static
    beq CODE_8A9A2F             ; $8A9A27 static
    jsr.w byte_buffer_push      ; $8A9A29 static
    inx                         ; $8A9A2C static
    bra CODE_8A9A23             ; $8A9A2D static
CODE_8A9A2F:
    rep.b #$20                  ; $8A9A2F static
    plx                         ; $8A9A31 static
    rts                         ; $8A9A32 static
byte_buffer_decimal:
    pha                         ; $8A9A33
    phx                         ; $8A9A34
    phy                         ; $8A9A35
    cmp.w #$0000                ; $8A9A36
    beq CODE_8A9A74             ; $8A9A39
    stz.b $5A                   ; $8A9A3B
    sta.b $5C                   ; $8A9A3D
    ldx.w #$0000                ; $8A9A3F
CODE_8A9A42:
    ldy.w #$0030                ; $8A9A42
CODE_8A9A45:
    sec                         ; $8A9A45
    sbc.w $9D83,x               ; $8A9A46
    bcc CODE_8A9A50             ; $8A9A49
    iny                         ; $8A9A4B
    sta.b $5C                   ; $8A9A4C
    bra CODE_8A9A45             ; $8A9A4E
CODE_8A9A50:
    inx                         ; $8A9A50
    inx                         ; $8A9A51
    lda.b $5A                   ; $8A9A52
    bne CODE_8A9A5F             ; $8A9A54
    lda.b $5C                   ; $8A9A56
    cpy.w #$0030                ; $8A9A58
    beq CODE_8A9A45             ; $8A9A5B
    inc.b $5A                   ; $8A9A5D
CODE_8A9A5F:
    tya                         ; $8A9A5F
    sep.b #$20                  ; $8A9A60
    jsr.w byte_buffer_push      ; $8A9A62
    rep.b #$20                  ; $8A9A65
    cpx.w #$000A                ; $8A9A67
    beq CODE_8A9A70             ; $8A9A6A
    lda.b $5C                   ; $8A9A6C
    bra CODE_8A9A42             ; $8A9A6E
CODE_8A9A70:
    ply                         ; $8A9A70
    plx                         ; $8A9A71
    pla                         ; $8A9A72
    rts                         ; $8A9A73
CODE_8A9A74:
    sep.b #$20                  ; $8A9A74
    lda.b #$30                  ; $8A9A76
    jsr.w byte_buffer_push      ; $8A9A78
    rep.b #$20                  ; $8A9A7B
    ply                         ; $8A9A7D
    plx                         ; $8A9A7E
    pla                         ; $8A9A7F
    rts                         ; $8A9A80
byte_buffer_decimal2:
    lda.b $5E                   ; $8A9A81
    cmp.w #$000A                ; $8A9A83
    bpl CODE_8A9A93             ; $8A9A86
    pha                         ; $8A9A88
    sep.b #$20                  ; $8A9A89
    lda.b #$20                  ; $8A9A8B
    jsr.w byte_buffer_push      ; $8A9A8D
    rep.b #$20                  ; $8A9A90
    pla                         ; $8A9A92
CODE_8A9A93:
    lda.b $5E                   ; $8A9A93
    jsr.w byte_buffer_decimal   ; $8A9A95
    rts                         ; $8A9A98
byte_buffer_space:
    pha                         ; $8A9A99
    sep.b #$20                  ; $8A9A9A
    lda.b #$20                  ; $8A9A9C
    jsr.w byte_buffer_push      ; $8A9A9E
    rep.b #$20                  ; $8A9AA1
    pla                         ; $8A9AA3
    rts                         ; $8A9AA4
results_text_build:
    stz.b $54                   ; $8A9AA5
    jsr.w variant_lines         ; $8A9AA7
    jsr.w player_names_put      ; $8A9AAA
    jsr.w text_lines_copy       ; $8A9AAD
    rts                         ; $8A9AB0
variant_lines:
    lda.l $8103DC               ; $8A9AB1
    and.w #$000F                ; $8A9AB5
    cmp.w #$0002                ; $8A9AB8
    beq CODE_8A9AC3             ; $8A9ABB
    lda.w #$0014                ; $8A9ABD
    sta.b $56                   ; $8A9AC0
    rts                         ; $8A9AC2
CODE_8A9AC3:
    lda.w #$0004                ; $8A9AC3 static
    sta.b $56                   ; $8A9AC6 static
    rts                         ; $8A9AC8 static
text_lines_copy:
    lda.b $56                   ; $8A9AC9
    beq CODE_8A9AF0             ; $8A9ACB
    ldy.w #$0000                ; $8A9ACD
CODE_8A9AD0:
    sep.b #$20                  ; $8A9AD0
    ldx.b $54                   ; $8A9AD2
CODE_8A9AD4:
    lda.w $9DEE,y               ; $8A9AD4
    sta.l $7F7918,x             ; $8A9AD7
    beq CODE_8A9AE1             ; $8A9ADB
    inx                         ; $8A9ADD
    iny                         ; $8A9ADE
    bra CODE_8A9AD4             ; $8A9ADF
CODE_8A9AE1:
    rep.b #$20                  ; $8A9AE1
    iny                         ; $8A9AE3
    clc                         ; $8A9AE4
    lda.b $54                   ; $8A9AE5
    adc.w #$0010                ; $8A9AE7
    sta.b $54                   ; $8A9AEA
    dec.b $56                   ; $8A9AEC
    bne CODE_8A9AD0             ; $8A9AEE
CODE_8A9AF0:
    rts                         ; $8A9AF0
player_names_put:
    lda.l $8103DC               ; $8A9AF1
    and.w #$000F                ; $8A9AF5
    cmp.w #$0000                ; $8A9AF8
    beq CODE_8A9B2D             ; $8A9AFB
    cmp.w #$0001                ; $8A9AFD static
    beq CODE_8A9B4A             ; $8A9B00 static
    cmp.w #$0002                ; $8A9B02 static
    beq CODE_8A9B08             ; $8A9B05 static
    rts                         ; $8A9B07 static
CODE_8A9B08:
    ldx.w #$03E4                ; $8A9B08 static
    ldy.w #$0081                ; $8A9B0B static
    jsr.w text_line_put         ; $8A9B0E static
    ldx.w #$03F1                ; $8A9B11 static
    ldy.w #$0081                ; $8A9B14 static
    jsr.w text_line_put         ; $8A9B17 static
    ldx.w #$03FE                ; $8A9B1A static
    ldy.w #$0081                ; $8A9B1D static
    jsr.w text_line_put         ; $8A9B20 static
    ldx.w #$040B                ; $8A9B23 static
    ldy.w #$0081                ; $8A9B26 static
    jsr.w text_line_put         ; $8A9B29 static
    rts                         ; $8A9B2C static
CODE_8A9B2D:
    ldx.w #$037E                ; $8A9B2D
    ldy.w #$0081                ; $8A9B30
    jsr.w text_line_put         ; $8A9B33
    rts                         ; $8A9B36
CODE_8A9B37:
    ldx.w #$037E                ; $8A9B37 static
    ldy.w #$0081                ; $8A9B3A static
    jsr.w text_line_put         ; $8A9B3D static
    ldx.w #$9B7D                ; $8A9B40 static
    ldy.w #$008A                ; $8A9B43 static
    jsr.w text_line_put         ; $8A9B46 static
    rts                         ; $8A9B49 static
CODE_8A9B4A:
    lda.l $8103DC               ; $8A9B4A static
    and.w #$0030                ; $8A9B4E static
    cmp.w #$0010                ; $8A9B51 static
    bne CODE_8A9B58             ; $8A9B54 static
    bra CODE_8A9B37             ; $8A9B56 static
CODE_8A9B58:
    lda.l $8103DC               ; $8A9B58 static
    lsr a                       ; $8A9B5C static
    lsr a                       ; $8A9B5D static
    lsr a                       ; $8A9B5E static
    lsr a                       ; $8A9B5F static
    lsr a                       ; $8A9B60 static
    bcs CODE_8A9B67             ; $8A9B61 static
CODE_8A9B63:
    lsr a                       ; $8A9B63 static
    bcs CODE_8A9B72             ; $8A9B64 static
CODE_8A9B66:
    rts                         ; $8A9B66 static
CODE_8A9B67:
    ldx.w #$037E                ; $8A9B67 static
    ldy.w #$0081                ; $8A9B6A static
    jsr.w text_line_put         ; $8A9B6D static
    bra CODE_8A9B63             ; $8A9B70 static
CODE_8A9B72:
    ldx.w #$03AD                ; $8A9B72 static
    ldy.w #$0081                ; $8A9B75 static
    jsr.w text_line_put         ; $8A9B78 static
    bra CODE_8A9B66             ; $8A9B7B static
    incbin !baserom:$51B7D..$51B85
text_line_put:
    pha                         ; $8A9B85
    stx.b $62                   ; $8A9B86
    sty.b $64                   ; $8A9B88
    sep.b #$20                  ; $8A9B8A
    ldx.b $54                   ; $8A9B8C
    phx                         ; $8A9B8E
    ldy.w #$0000                ; $8A9B8F
CODE_8A9B92:
    lda.b [$62],y               ; $8A9B92
    sta.l $7F7918,x             ; $8A9B94
    beq CODE_8A9BA0             ; $8A9B98
    inx                         ; $8A9B9A
    iny                         ; $8A9B9B
    inc.b $54                   ; $8A9B9C
    bra CODE_8A9B92             ; $8A9B9E
CODE_8A9BA0:
    plx                         ; $8A9BA0
    stx.b $54                   ; $8A9BA1
    rep.b #$20                  ; $8A9BA3
    jsr.w func_8A9BAA           ; $8A9BA5
    pla                         ; $8A9BA8
    rts                         ; $8A9BA9
func_8A9BAA:
    clc                         ; $8A9BAA
    lda.b $54                   ; $8A9BAB
    adc.w #$0010                ; $8A9BAD
    sta.b $54                   ; $8A9BB0
    dec.b $56                   ; $8A9BB2
    rts                         ; $8A9BB4
func_8A9BB5:
    phy                         ; $8A9BB5
CODE_8A9BB6:
    ldx.w #$0000                ; $8A9BB6
CODE_8A9BB9:
    lda.l $7F7FBC,x             ; $8A9BB9
    bne CODE_8A9BC7             ; $8A9BBD
    lda.b $68                   ; $8A9BBF
    cmp.l $81041C,x             ; $8A9BC1
    beq CODE_8A9BD5             ; $8A9BC5
CODE_8A9BC7:
    inx                         ; $8A9BC7
    inx                         ; $8A9BC8
    cpx.b $66                   ; $8A9BC9
    bne CODE_8A9BB9             ; $8A9BCB
    lda.b $68                   ; $8A9BCD
    beq CODE_8A9BDF             ; $8A9BCF
    dec.b $68                   ; $8A9BD1
    bra CODE_8A9BB6             ; $8A9BD3
CODE_8A9BD5:
    lda.w #$FFFF                ; $8A9BD5
    sta.l $7F7FBC,x             ; $8A9BD8
    ply                         ; $8A9BDC
    clc                         ; $8A9BDD
    rts                         ; $8A9BDE
CODE_8A9BDF:
    ply                         ; $8A9BDF
    sec                         ; $8A9BE0
    rts                         ; $8A9BE1
func_8A9BE2:
    stz.b $68                   ; $8A9BE2
    ldx.w #$0026                ; $8A9BE4
CODE_8A9BE7:
    lda.l $81041C,x             ; $8A9BE7
    cmp.b $68                   ; $8A9BEB
    bmi CODE_8A9BF1             ; $8A9BED
    sta.b $68                   ; $8A9BEF
CODE_8A9BF1:
    dex                         ; $8A9BF1
    dex                         ; $8A9BF2
    bpl CODE_8A9BE7             ; $8A9BF3
    rts                         ; $8A9BF5
text_map_clear:
    stz.b $60                   ; $8A9BF6
    stz.b $14                   ; $8A9BF8
    stz.b $58                   ; $8A9BFA
    ldx.w #$0140                ; $8A9BFC
    lda.w #$0000                ; $8A9BFF
CODE_8A9C02:
    sta.l $7F7918,x             ; $8A9C02
    dex                         ; $8A9C06
    dex                         ; $8A9C07
    bpl CODE_8A9C02             ; $8A9C08
    ldx.w #$0026                ; $8A9C0A
    lda.w #$0000                ; $8A9C0D
CODE_8A9C10:
    sta.l $7F7FBC,x             ; $8A9C10
    dex                         ; $8A9C14
    dex                         ; $8A9C15
    bpl CODE_8A9C10             ; $8A9C16
    rtl                         ; $8A9C18
byte_buffer_push:
    phx                         ; $8A9C19
    ldx.b $58                   ; $8A9C1A
    sta.l $7F7D00,x             ; $8A9C1C
    inx                         ; $8A9C20
    stx.b $58                   ; $8A9C21
    plx                         ; $8A9C23
    inc.b $60                   ; $8A9C24
    rts                         ; $8A9C26
hdma_table_0d00_step:
    inc.b $6C                   ; $8A9C27
    lda.b $6C                   ; $8A9C29
    and.w #$0003                ; $8A9C2B
    beq CODE_8A9C31             ; $8A9C2E
    rts                         ; $8A9C30
CODE_8A9C31:
    ldx.b $6E                   ; $8A9C31
    cpx.w #$002A                ; $8A9C33
    beq CODE_8A9C47             ; $8A9C36
    lda.w $0D01,x               ; $8A9C38
    cmp.w #$00E0                ; $8A9C3B
    beq CODE_8A9C43             ; $8A9C3E
    dec.w $0D01,x               ; $8A9C40
CODE_8A9C43:
    inx                         ; $8A9C43
    inx                         ; $8A9C44
    stx.b $6E                   ; $8A9C45
CODE_8A9C47:
    rts                         ; $8A9C47
hdma_table_0d00_settle:
    jsr.w hdma_table_0d00_step  ; $8A9C48
    sep.b #$20                  ; $8A9C4B
    ldy.w #$0015                ; $8A9C4D
    ldx.w #$0000                ; $8A9C50
CODE_8A9C53:
    lda.w $0D01,x               ; $8A9C53
    cmp.b #$FF                  ; $8A9C56
    beq CODE_8A9C61             ; $8A9C58
    cmp.b #$E0                  ; $8A9C5A
    beq CODE_8A9C61             ; $8A9C5C
    dec.w $0D01,x               ; $8A9C5E
CODE_8A9C61:
    inx                         ; $8A9C61
    inx                         ; $8A9C62
    dey                         ; $8A9C63
    bne CODE_8A9C53             ; $8A9C64
    rep.b #$20                  ; $8A9C66
    rts                         ; $8A9C68
hdma_coldata_setup:
    jsr.w hdma_table_0d00_init  ; $8A9C69
    sep.b #$10                  ; $8A9C6C
    ldx.b #$32                  ; $8A9C6E
    stx.w $4321                 ; $8A9C70
    ldx.b #$00                  ; $8A9C73
    stx.w $4320                 ; $8A9C75
    lda.w #$0D00                ; $8A9C78
    sta.w $4322                 ; $8A9C7B
    ldx.b #$81                  ; $8A9C7E
    stx.w $4324                 ; $8A9C80
    rep.b #$10                  ; $8A9C83
    rts                         ; $8A9C85
hdma_table_0d00_init:
    sep.b #$20                  ; $8A9C86
    lda.b #$36                  ; $8A9C88
    sta.w $0D00                 ; $8A9C8A
    lda.b #$FF                  ; $8A9C8D
    sta.w $0D01                 ; $8A9C8F
    ldy.w #$0015                ; $8A9C92
    ldx.w #$0000                ; $8A9C95
CODE_8A9C98:
    lda.b #$08                  ; $8A9C98
    sta.w $0D02,x               ; $8A9C9A
    lda.b #$FF                  ; $8A9C9D
    sta.w $0D03,x               ; $8A9C9F
    inx                         ; $8A9CA2
    inx                         ; $8A9CA3
    dey                         ; $8A9CA4
    bne CODE_8A9C98             ; $8A9CA5
    stz.w $0D02,x               ; $8A9CA7
    stz.w $0D03,x               ; $8A9CAA
    rep.b #$20                  ; $8A9CAD
    rts                         ; $8A9CAF
player_in_top5:
    jsr.w slot_order_fill       ; $8A9CB0
    lda.l $8103DC               ; $8A9CB3
    bit.w #$000F                ; $8A9CB7
    beq CODE_8A9CC6             ; $8A9CBA
    and.w #$0030                ; $8A9CBC
    cmp.w #$0010                ; $8A9CBF
    beq CODE_8A9CC6             ; $8A9CC2
    incbin !baserom:$51CC4..$51CC6
CODE_8A9CC6:
    ldx.w #$0000                ; $8A9CC6
CODE_8A9CC9:
    lda.w $0D00,x               ; $8A9CC9
    beq CODE_8A9CF3             ; $8A9CCC
    inx                         ; $8A9CCE static
    inx                         ; $8A9CCF static
    cpx.w #$000A                ; $8A9CD0 static
    bne CODE_8A9CC9             ; $8A9CD3 static
    jsr.w clear_041c            ; $8A9CD5 static
    sec                         ; $8A9CD8 static
    rtl                         ; $8A9CD9 static
    ldx.w #$0000                ; $8A9CDA static
CODE_8A9CDD:
    lda.w $0D00,x               ; $8A9CDD static
    beq CODE_8A9CF3             ; $8A9CE0 static
    cmp.w #$0001                ; $8A9CE2 static
    beq CODE_8A9CF3             ; $8A9CE5 static
    inx                         ; $8A9CE7 static
    inx                         ; $8A9CE8 static
    cpx.w #$000A                ; $8A9CE9 static
    bne CODE_8A9CDD             ; $8A9CEC static
    jsr.w clear_041c            ; $8A9CEE static
    sec                         ; $8A9CF1 static
    rtl                         ; $8A9CF2 static
CODE_8A9CF3:
    jsr.w clear_041c            ; $8A9CF3
    clc                         ; $8A9CF6
    rtl                         ; $8A9CF7
slot_order_fill:
    ldx.w #$0026                ; $8A9CF8
    lda.w #$0000                ; $8A9CFB
CODE_8A9CFE:
    sta.l $7F7FBC,x             ; $8A9CFE
    dex                         ; $8A9D02
    dex                         ; $8A9D03
    bpl CODE_8A9CFE             ; $8A9D04
    jsr.w func_8A9BE2           ; $8A9D06
    jsr.w func_8A99D0           ; $8A9D09
    ldy.w #$0000                ; $8A9D0C
CODE_8A9D0F:
    jsr.w func_8A9BB5           ; $8A9D0F
    bcs CODE_8A9D1D             ; $8A9D12
    txa                         ; $8A9D14
    lsr a                       ; $8A9D15
    sta.w $0D00,y               ; $8A9D16
    iny                         ; $8A9D19
    iny                         ; $8A9D1A
    bra CODE_8A9D0F             ; $8A9D1B
CODE_8A9D1D:
    rts                         ; $8A9D1D
clear_041c:
    ldx.w #$0026                ; $8A9D1E
    lda.w #$0000                ; $8A9D21
CODE_8A9D24:
    sta.l $81041C,x             ; $8A9D24
    dex                         ; $8A9D28
    dex                         ; $8A9D29
    bpl CODE_8A9D24             ; $8A9D2A
    rts                         ; $8A9D2C
    incbin !baserom:$51D2D..$51E5B
options_state_clear:
    phx                         ; $8A9E5B
    ldx.w #$00FE                ; $8A9E5C
CODE_8A9E5F:
    stz.w $0D00,x               ; $8A9E5F
    dex                         ; $8A9E62
    dex                         ; $8A9E63
    bpl CODE_8A9E5F             ; $8A9E64
    plx                         ; $8A9E66
    cpx.w #$0000                ; $8A9E67
    beq options_screen_open     ; $8A9E6A
    cpx.w #$0002                ; $8A9E6C
    beq CODE_8A9E7A             ; $8A9E6F
    cpx.w #$0003                ; $8A9E71
    db $F0,$08 ; beq            ; $8A9E74
    jsr.w password_show_open    ; $8A9E76
    rtl                         ; $8A9E79
CODE_8A9E7A:
    jsr.w title_card_show       ; $8A9E7A
    rtl                         ; $8A9E7D
    incbin !baserom:$51E7E..$51E82
options_screen_open:
    lda.w $0008                 ; $8A9E82
    pha                         ; $8A9E85
    stz.w $0196                 ; $8A9E86
    stz.w $0198                 ; $8A9E89
    stz.w $019A                 ; $8A9E8C
    jsl.l clear_dp_scratch      ; $8A9E8F
    ldx.w #$00FE                ; $8A9E93
CODE_8A9E96:
    stz.w $0D00,x               ; $8A9E96
    dex                         ; $8A9E99
    dex                         ; $8A9E9A
    bpl CODE_8A9E96             ; $8A9E9B
    lda.w $03DC                 ; $8A9E9D
    and.w #$DFFF                ; $8A9EA0
    sta.w $03DC                 ; $8A9EA3
    stz.b $8E                   ; $8A9EA6
    ldx.w #$C4B0                ; $8A9EA8
    lda.w #$008A                ; $8A9EAB
    jsl.l video_load_screen_long; $8A9EAE
    jsr.w hdma_window_setup     ; $8A9EB2
    jsl.l clear_vram            ; $8A9EB5
    jsr.w func_8ABA55           ; $8A9EB9
    jsr.w func_8AB994           ; $8A9EBC
    jsr.w text_tiles_put_0      ; $8A9EBF
    jsr.w wipe_columns          ; $8A9EC2
options_screen_setup:
    jsl.l func_8A8EEF           ; $8A9EC5
    lda.w #$FC5F                ; $8A9EC9
    sta.w $0008                 ; $8A9ECC
    sep.b #$20                  ; $8A9ECF
    lda.b #$82                  ; $8A9ED1
    sta.w $420C                 ; $8A9ED3
    lda.b #$81                  ; $8A9ED6
    sta.w $4200                 ; $8A9ED8
    rep.b #$20                  ; $8A9EDB
    stz.b $3A                   ; $8A9EDD
    stz.b $3C                   ; $8A9EDF
    stz.w $0D88,x               ; $8A9EE1
    lda.w #$0004                ; $8A9EE4
    sta.b $7A                   ; $8A9EE7
    sta.b $7C                   ; $8A9EE9
    lda.w #$0007                ; $8A9EEB
    sta.b $7E                   ; $8A9EEE
    inc.b $8E                   ; $8A9EF0
    lda.w #$0F00                ; $8A9EF2
    sta.w $0198                 ; $8A9EF5
    lda.w #$0080                ; $8A9EF8
    sta.w $0196                 ; $8A9EFB
    ldx.w #$000A                ; $8A9EFE
    ldy.w #$0010                ; $8A9F01
    stx.b $3A                   ; $8A9F04
    sty.b $3C                   ; $8A9F06
    ldx.w #$0000                ; $8A9F08
    ldy.w #$001F                ; $8A9F0B
    stx.b $3E                   ; $8A9F0E
    sty.b $40                   ; $8A9F10
    lda.w #$007C                ; $8A9F12
    sta.w $0D60                 ; $8A9F15
    lda.w #$007C                ; $8A9F18
    sta.w $0D68                 ; $8A9F1B
    lda.w #$0040                ; $8A9F1E
    sta.w $0D78                 ; $8A9F21
    lda.w #$0008                ; $8A9F24
    sta.b $4C                   ; $8A9F27
    lda.w #$CAB0                ; $8A9F29
    jsr.w func_8AA4BE           ; $8A9F2C
    stz.b $6C                   ; $8A9F2F
    jsr.w variant_flags_apply   ; $8A9F31
    lda.w #$0A8C                ; $8A9F34
    sta.w $0D7E                 ; $8A9F37
CODE_8A9F3A:
    lda.w #$0001                ; $8A9F3A
    jsl.l wait_frames_long      ; $8A9F3D
    lda.w #$CBA7                ; $8A9F41
    jsr.w menu_draw             ; $8A9F44
options_screen_tick:
    jsr.w timer_0d7e_tick       ; $8A9F47
    lda.b $6C                   ; $8A9F4A
    beq CODE_8A9F3A             ; $8A9F4C
    jsr.w flag_03dc_from_52     ; $8A9F4E
    stz.w $0198                 ; $8A9F51
    lda.w #$FFC0                ; $8A9F54
    sta.w $0196                 ; $8A9F57
CODE_8A9F5A:
    lda.w $019A                 ; $8A9F5A
    bne CODE_8A9F5A             ; $8A9F5D
    pla                         ; $8A9F5F
    sta.w $0008                 ; $8A9F60
    rtl                         ; $8A9F63
flag_03dc_from_52:
    lda.b $52                   ; $8A9F64
    beq CODE_8A9F6F             ; $8A9F66
    lda.w #$1000                ; $8A9F68 static
    tsb.w $03DC                 ; $8A9F6B static
    rts                         ; $8A9F6E static
CODE_8A9F6F:
    lda.w #$1000                ; $8A9F6F
    trb.w $03DC                 ; $8A9F72
    rts                         ; $8A9F75
timer_0d7e_tick:
    dec.w $0D7E                 ; $8A9F76
    bne CODE_8A9FA0             ; $8A9F79
    lda.w #$2000                ; $8A9F7B
    sta.w $03DC                 ; $8A9F7E
    jsr.w rng_next_8a_2         ; $8A9F81
    and.w #$0003                ; $8A9F84
    cmp.w #$0003                ; $8A9F87
    bne CODE_8A9F8D             ; $8A9F8A
    dec a                       ; $8A9F8C
CODE_8A9F8D:
    ora.w $03DC                 ; $8A9F8D
    sta.w $03DC                 ; $8A9F90
    lda.w $03DC                 ; $8A9F93
    and.w #$0007                ; $8A9F96
    cmp.w #$0002                ; $8A9F99
    bne CODE_8A9F9E             ; $8A9F9C
CODE_8A9F9E:
    inc.b $6C                   ; $8A9F9E
CODE_8A9FA0:
    rts                         ; $8A9FA0
func_8A9FA1:
    phx                         ; $8A9FA1
    phy                         ; $8A9FA2
    ldx.w #$0000                ; $8A9FA3
    lda.b $58                   ; $8A9FA6
    asl a                       ; $8A9FA8
    tay                         ; $8A9FA9
    lda.w $9FF9,y               ; $8A9FAA
CODE_8A9FAD:
    lsr a                       ; $8A9FAD
    bcc CODE_8A9FCA             ; $8A9FAE
    pha                         ; $8A9FB0
    phx                         ; $8A9FB1
    lda.w $9FE9,x               ; $8A9FB2
    cmp.w #$FFFF                ; $8A9FB5
    beq CODE_8A9FE4             ; $8A9FB8
    tax                         ; $8A9FBA
    lda.w $1516,x               ; $8A9FBB
    and.w #$FF7F                ; $8A9FBE
    sta.w $1516,x               ; $8A9FC1
    plx                         ; $8A9FC4
    pla                         ; $8A9FC5
    inx                         ; $8A9FC6
    inx                         ; $8A9FC7
    bra CODE_8A9FAD             ; $8A9FC8
CODE_8A9FCA:
    pha                         ; $8A9FCA
    phx                         ; $8A9FCB
    lda.w $9FE9,x               ; $8A9FCC
    cmp.w #$FFFF                ; $8A9FCF
    beq CODE_8A9FE4             ; $8A9FD2
    tax                         ; $8A9FD4
    lda.w $1516,x               ; $8A9FD5
    ora.w #$0080                ; $8A9FD8
    sta.w $1516,x               ; $8A9FDB
    plx                         ; $8A9FDE
    pla                         ; $8A9FDF
    inx                         ; $8A9FE0
    inx                         ; $8A9FE1
    bra CODE_8A9FAD             ; $8A9FE2
CODE_8A9FE4:
    ply                         ; $8A9FE4
    plx                         ; $8A9FE5
    ply                         ; $8A9FE6
    plx                         ; $8A9FE7
    rts                         ; $8A9FE8
    incbin !baserom:$51FE9..$52005
func_8AA005:
    stz.b $56                   ; $8AA005
    stz.b $58                   ; $8AA007
    lda.w $0018                 ; $8AA009
    jsr.w func_8AA022           ; $8AA00C
    lda.w $0014                 ; $8AA00F
    jsr.w func_8AA022           ; $8AA012
    lda.w $0010                 ; $8AA015
    jsr.w func_8AA022           ; $8AA018
    lda.w $000C                 ; $8AA01B
    jsr.w func_8AA022           ; $8AA01E
    rts                         ; $8AA021
func_8AA022:
    eor.w #$0001                ; $8AA022
    and.w #$0001                ; $8AA025
    bne CODE_8AA033             ; $8AA028
    clc                         ; $8AA02A
    rol.b $56                   ; $8AA02B
    clc                         ; $8AA02D
    adc.b $58                   ; $8AA02E
    sta.b $58                   ; $8AA030
    rts                         ; $8AA032
CODE_8AA033:
    sec                         ; $8AA033
    rol.b $56                   ; $8AA034
    clc                         ; $8AA036
    adc.b $58                   ; $8AA037
    sta.b $58                   ; $8AA039
    rts                         ; $8AA03B
    incbin !baserom:$5203C..$52050
func_8AA050:
    lda.b $8E                   ; $8AA050
    beq CODE_8AA069             ; $8AA052
    jsr.w upload_7f_to_3000     ; $8AA054
    jsl.l oam_from_objects      ; $8AA057
    inc.w $0164                 ; $8AA05B
    lda.w $0164                 ; $8AA05E
    and.w #$0007                ; $8AA061
    bne CODE_8AA069             ; $8AA064
    inc.w $0166                 ; $8AA066
CODE_8AA069:
    jsr.w func_8AA005           ; $8AA069
    rtl                         ; $8AA06C
hdma_window_setup:
    jsr.w hdma_window_tables_init; $8AA06D
    sep.b #$10                  ; $8AA070
    ldx.b #$0F                  ; $8AA072
    stx.w $4371                 ; $8AA074
    ldx.b #$02                  ; $8AA077
    stx.w $4370                 ; $8AA079
    lda.w #$0D00                ; $8AA07C
    sta.w $4372                 ; $8AA07F
    ldx.b #$81                  ; $8AA082
    stx.w $4374                 ; $8AA084
    ldx.b #$32                  ; $8AA087
    stx.w $4311                 ; $8AA089
    ldx.b #$00                  ; $8AA08C
    stx.w $4310                 ; $8AA08E
    lda.w #$0D20                ; $8AA091
    sta.w $4312                 ; $8AA094
    ldx.b #$81                  ; $8AA097
    stx.w $4314                 ; $8AA099
    rep.b #$10                  ; $8AA09C
    rts                         ; $8AA09E
hdma_window_tables_init:
    sep.b #$20                  ; $8AA09F
    lda.b #$4C                  ; $8AA0A1
    sta.w $0D00                 ; $8AA0A3
    lda.b #$18                  ; $8AA0A6
    sta.w $0D03                 ; $8AA0A8
    sta.w $0D06                 ; $8AA0AB
    sta.w $0D09                 ; $8AA0AE
    sta.w $0D0C                 ; $8AA0B1
    sta.w $0D0F                 ; $8AA0B4
    sta.w $0D12                 ; $8AA0B7
    sta.w $0D15                 ; $8AA0BA
    stz.w $0D18                 ; $8AA0BD
    lda.b #$4C                  ; $8AA0C0
    sta.w $0D20                 ; $8AA0C2
    lda.b #$18                  ; $8AA0C5
    sta.w $0D22                 ; $8AA0C7
    sta.w $0D24                 ; $8AA0CA
    sta.w $0D26                 ; $8AA0CD
    sta.w $0D28                 ; $8AA0D0
    sta.w $0D2A                 ; $8AA0D3
    sta.w $0D2C                 ; $8AA0D6
    sta.w $0D2E                 ; $8AA0D9
    stz.w $0D30                 ; $8AA0DC
    lda.b #$E0                  ; $8AA0DF
    sta.w $0D23                 ; $8AA0E1
    sta.w $0D25                 ; $8AA0E4
    sta.w $0D27                 ; $8AA0E7
    sta.w $0D29                 ; $8AA0EA
    sta.w $0D2B                 ; $8AA0ED
    sta.w $0D2D                 ; $8AA0F0
    sta.w $0D2F                 ; $8AA0F3
    rep.b #$20                  ; $8AA0F6
    rts                         ; $8AA0F8
sprite_frame:
    phx                         ; $8AA0F9
    ldy.b $7E                   ; $8AA0FA
    jsr.w step_toward_0d70      ; $8AA0FC
    ldy.b $7C                   ; $8AA0FF
    jsr.w step_toward_0d68      ; $8AA101
    ldy.b $7A                   ; $8AA104
    jsr.w step_toward_0d80      ; $8AA106
    lda.w $0D60,x               ; $8AA109
    pha                         ; $8AA10C
    lda.w $0D68,x               ; $8AA10D
    pha                         ; $8AA110
    lda.w $0D70,x               ; $8AA111
    pha                         ; $8AA114
    lda.w $0D70,x               ; $8AA115
    lsr a                       ; $8AA118
    sta.b $48                   ; $8AA119
    lda.w $0D60,x               ; $8AA11B
    sec                         ; $8AA11E
    sbc.b $48                   ; $8AA11F
    clc                         ; $8AA121
    adc.w $0D80,x               ; $8AA122
    sta.w $0D60,x               ; $8AA125
    jsr.w take_slot             ; $8AA128
    lda.w #$FFFF                ; $8AA12B
    sta.w $0500,y               ; $8AA12E
    lda.w #$3000                ; $8AA131
    sta.w $0800,y               ; $8AA134
    lda.w $0D60,x               ; $8AA137
    sta.w $0600,y               ; $8AA13A
    lda.w $0D68,x               ; $8AA13D
    sta.w $0700,y               ; $8AA140
    jsr.w take_slot             ; $8AA143
    lda.w #$FFFF                ; $8AA146
    sta.w $0500,y               ; $8AA149
    lda.w #$3001                ; $8AA14C
    sta.w $0800,y               ; $8AA14F
    lda.w $0D60,x               ; $8AA152
    sta.w $0600,y               ; $8AA155
    lda.w $0D68,x               ; $8AA158
    clc                         ; $8AA15B
    adc.b $4C                   ; $8AA15C
    sta.w $0700,y               ; $8AA15E
    jsr.w take_slot             ; $8AA161
    lda.w #$FFFF                ; $8AA164
    sta.w $0500,y               ; $8AA167
    lda.w #$B000                ; $8AA16A
    sta.w $0800,y               ; $8AA16D
    lda.w $0D60,x               ; $8AA170
    sta.w $0600,y               ; $8AA173
    lda.w $0D68,x               ; $8AA176
    clc                         ; $8AA179
    adc.b $4C                   ; $8AA17A
    clc                         ; $8AA17C
    adc.b $4C                   ; $8AA17D
    sta.w $0700,y               ; $8AA17F
    lda.w $0D60,x               ; $8AA182
    clc                         ; $8AA185
    adc.w #$0008                ; $8AA186
    sta.w $0D60,x               ; $8AA189
    ldy.w #$0000                ; $8AA18C
CODE_8AA18F:
    sec                         ; $8AA18F
    lda.w $0D70,x               ; $8AA190
    sbc.w #$0008                ; $8AA193
    bcc CODE_8AA1A0             ; $8AA196
    sta.w $0D70,x               ; $8AA198
    jsr.w func_8AA216           ; $8AA19B
    bra CODE_8AA18F             ; $8AA19E
CODE_8AA1A0:
    adc.w #$0008                ; $8AA1A0
    clc                         ; $8AA1A3
    adc.w $0D60,x               ; $8AA1A4
    sec                         ; $8AA1A7
    sbc.w #$0008                ; $8AA1A8
    sta.w $0D60,x               ; $8AA1AB
    jsr.w take_slot             ; $8AA1AE
    lda.w #$FFFF                ; $8AA1B1
    sta.w $0500,y               ; $8AA1B4
    lda.w #$7000                ; $8AA1B7
    sta.w $0800,y               ; $8AA1BA
    lda.w $0D60,x               ; $8AA1BD
    sta.w $0600,y               ; $8AA1C0
    lda.w $0D68,x               ; $8AA1C3
    sta.w $0700,y               ; $8AA1C6
    jsr.w take_slot             ; $8AA1C9
    lda.w #$FFFF                ; $8AA1CC
    sta.w $0500,y               ; $8AA1CF
    lda.w #$7001                ; $8AA1D2
    sta.w $0800,y               ; $8AA1D5
    lda.w $0D60,x               ; $8AA1D8
    sta.w $0600,y               ; $8AA1DB
    lda.w $0D68,x               ; $8AA1DE
    clc                         ; $8AA1E1
    adc.b $4C                   ; $8AA1E2
    sta.w $0700,y               ; $8AA1E4
    jsr.w take_slot             ; $8AA1E7
    lda.w #$FFFF                ; $8AA1EA
    sta.w $0500,y               ; $8AA1ED
    lda.w #$F000                ; $8AA1F0
    sta.w $0800,y               ; $8AA1F3
    lda.w $0D60,x               ; $8AA1F6
    sta.w $0600,y               ; $8AA1F9
    lda.w $0D68,x               ; $8AA1FC
    clc                         ; $8AA1FF
    adc.b $4C                   ; $8AA200
    clc                         ; $8AA202
    adc.b $4C                   ; $8AA203
    sta.w $0700,y               ; $8AA205
    pla                         ; $8AA208
    sta.w $0D70,x               ; $8AA209
    pla                         ; $8AA20C
    sta.w $0D68,x               ; $8AA20D
    pla                         ; $8AA210
    sta.w $0D60,x               ; $8AA211
    plx                         ; $8AA214
    rts                         ; $8AA215
func_8AA216:
    jsr.w take_slot             ; $8AA216
    lda.w #$FFFF                ; $8AA219
    sta.w $0500,y               ; $8AA21C
    lda.w $0D60,x               ; $8AA21F
    sta.w $0600,y               ; $8AA222
    lda.w $0D68,x               ; $8AA225
    sta.w $0700,y               ; $8AA228
    lda.w #$3002                ; $8AA22B
    sta.w $0800,y               ; $8AA22E
    jsr.w take_slot             ; $8AA231
    lda.w #$FFFF                ; $8AA234
    sta.w $0500,y               ; $8AA237
    lda.w $0D60,x               ; $8AA23A
    sta.w $0600,y               ; $8AA23D
    lda.w $0D68,x               ; $8AA240
    clc                         ; $8AA243
    adc.b $4C                   ; $8AA244
    clc                         ; $8AA246
    adc.b $4C                   ; $8AA247
    sta.w $0700,y               ; $8AA249
    lda.w #$B002                ; $8AA24C
    sta.w $0800,y               ; $8AA24F
    clc                         ; $8AA252
    lda.w $0D60,x               ; $8AA253
    adc.w #$0008                ; $8AA256
    sta.w $0D60,x               ; $8AA259
    rts                         ; $8AA25C
take_slot:
    ldy.b $8C                   ; $8AA25D
    inc.b $8C                   ; $8AA25F
    inc.b $8C                   ; $8AA261
    rts                         ; $8AA263
step_toward_0d80:
    lda.w $0D88,x               ; $8AA264
    sec                         ; $8AA267
    sbc.w $0D80,x               ; $8AA268
    beq CODE_8AA27C             ; $8AA26B
    bmi CODE_8AA276             ; $8AA26D
    inc.w $0D80,x               ; $8AA26F
    dey                         ; $8AA272
    bne step_toward_0d80        ; $8AA273
    rts                         ; $8AA275
CODE_8AA276:
    dec.w $0D80,x               ; $8AA276
    dey                         ; $8AA279
    bne step_toward_0d80        ; $8AA27A
CODE_8AA27C:
    rts                         ; $8AA27C
step_toward_0d70:
    lda.w $0D78,x               ; $8AA27D
    sec                         ; $8AA280
    sbc.w $0D70,x               ; $8AA281
    beq CODE_8AA295             ; $8AA284
    bmi CODE_8AA28F             ; $8AA286
    inc.w $0D70,x               ; $8AA288
    dey                         ; $8AA28B
    bne step_toward_0d70        ; $8AA28C
    rts                         ; $8AA28E
CODE_8AA28F:
    dec.w $0D70,x               ; $8AA28F
    dey                         ; $8AA292
    bne step_toward_0d70        ; $8AA293
CODE_8AA295:
    rts                         ; $8AA295
step_toward_0d68:
    lda.w $0D90,x               ; $8AA296
    sec                         ; $8AA299
    sbc.w $0D68,x               ; $8AA29A
    beq CODE_8AA2AE             ; $8AA29D
    bmi CODE_8AA2A8             ; $8AA29F
    inc.w $0D68,x               ; $8AA2A1
    dey                         ; $8AA2A4
    bne step_toward_0d68        ; $8AA2A5
    rts                         ; $8AA2A7
CODE_8AA2A8:
    dec.w $0D68,x               ; $8AA2A8
    dey                         ; $8AA2AB
    bne step_toward_0d68        ; $8AA2AC
CODE_8AA2AE:
    rts                         ; $8AA2AE
menu_draw:
    sta.b $4A                   ; $8AA2AF
    jsr.w clear_object_slots    ; $8AA2B1
    ldx.w #$0000                ; $8AA2B4
    jsr.w sprite_frame          ; $8AA2B7
    ldy.w #$0000                ; $8AA2BA
    lda.b ($4A),y               ; $8AA2BD
    sta.b $3C                   ; $8AA2BF
    inc.b $4A                   ; $8AA2C1
    inc.b $4A                   ; $8AA2C3
    inc.b $4A                   ; $8AA2C5
    inc.b $4A                   ; $8AA2C7
    ldy.w #$0004                ; $8AA2C9
    lda.b ($4A),y               ; $8AA2CC
    tax                         ; $8AA2CE
    ldy.w #$0000                ; $8AA2CF
    lda.b ($4A),y               ; $8AA2D2
    beq CODE_8AA32A             ; $8AA2D4
    jsr.w func_8AA32B           ; $8AA2D6
    lda.w $1516,x               ; $8AA2D9
    and.w #$0002                ; $8AA2DC
    beq CODE_8AA2F7             ; $8AA2DF
    incbin !baserom:$522E1..$522F7
CODE_8AA2F7:
    stz.b $68                   ; $8AA2F7
    lda.b ($4A),y               ; $8AA2F9
    jsr.w string_list_nth       ; $8AA2FB
    jsr.w text_print_centered   ; $8AA2FE
    jsr.w text_cursor_down3     ; $8AA301
    lda.w $1516,x               ; $8AA304
    and.w #$0001                ; $8AA307
    beq CODE_8AA31F             ; $8AA30A
    jsr.w func_8AA469           ; $8AA30C
    jsr.w func_8AA381           ; $8AA30F
    lda.w $1516,x               ; $8AA312
    and.w #$0080                ; $8AA315
    bne CODE_8AA31F             ; $8AA318
    jsr.w menu_select           ; $8AA31A
menu_draw_rest:
    bcs CODE_8AA32A             ; $8AA31D
CODE_8AA31F:
    clc                         ; $8AA31F
    lda.b $4A                   ; $8AA320
    adc.w #$0008                ; $8AA322
    sta.b $4A                   ; $8AA325
    jmp.w $A2C9                 ; $8AA327
CODE_8AA32A:
    rts                         ; $8AA32A
func_8AA32B:
    phy                         ; $8AA32B
    lda.w #$0000                ; $8AA32C
    sep.b #$20                  ; $8AA32F
    ldy.w #$0000                ; $8AA331
    lda.b $3C                   ; $8AA334
    sec                         ; $8AA336
    sbc.b #$0A                  ; $8AA337
    tay                         ; $8AA339
    lda.w $C599,y               ; $8AA33A
    tay                         ; $8AA33D
    lda.w $1516,x               ; $8AA33E
    and.b #$80                  ; $8AA341
    beq CODE_8AA34C             ; $8AA343
    lda.b #$E0                  ; $8AA345
    sta.w $0D23,y               ; $8AA347
    bra CODE_8AA376             ; $8AA34A
CODE_8AA34C:
    lda.w $1516,x               ; $8AA34C
    and.b #$40                  ; $8AA34F
    bne CODE_8AA35A             ; $8AA351
    lda.b #$E7                  ; $8AA353
    sta.w $0D23,y               ; $8AA355
    bra CODE_8AA376             ; $8AA358
CODE_8AA35A:
    phy                         ; $8AA35A
    lda.b $6E                   ; $8AA35B
    and.b #$1F                  ; $8AA35D
    tay                         ; $8AA35F
    lda.w $C5B7,y               ; $8AA360
    ply                         ; $8AA363
    pha                         ; $8AA364
    lda.w $0D23,y               ; $8AA365
    and.b #$E0                  ; $8AA368
    sta.w $0D23,y               ; $8AA36A
    pla                         ; $8AA36D
    ora.w $0D23,y               ; $8AA36E
    sta.w $0D23,y               ; $8AA371
    inc.b $6E                   ; $8AA374
CODE_8AA376:
    rep.b #$20                  ; $8AA376
    ply                         ; $8AA378
    rts                         ; $8AA379
text_cursor_down3:
    inc.b $3C                   ; $8AA37A
    inc.b $3C                   ; $8AA37C
    inc.b $3C                   ; $8AA37E
    rts                         ; $8AA380
func_8AA381:
    lda.w #$0800                ; $8AA381
    trb.w $000E                 ; $8AA384
    bne CODE_8AA3C2             ; $8AA387
    lda.w #$0800                ; $8AA389
    trb.w $0012                 ; $8AA38C
    bne CODE_8AA3C2             ; $8AA38F
    lda.w #$0800                ; $8AA391
    trb.w $0016                 ; $8AA394
    bne CODE_8AA3C2             ; $8AA397
    lda.w #$0800                ; $8AA399
    trb.w $001A                 ; $8AA39C
    bne CODE_8AA3C2             ; $8AA39F
    lda.w #$0400                ; $8AA3A1
    trb.w $000E                 ; $8AA3A4
    bne CODE_8AA3F4             ; $8AA3A7
    lda.w #$0400                ; $8AA3A9
    trb.w $0012                 ; $8AA3AC
    bne CODE_8AA3F4             ; $8AA3AF
    lda.w #$0400                ; $8AA3B1
    trb.w $0016                 ; $8AA3B4
    bne CODE_8AA3F4             ; $8AA3B7
    lda.w #$0400                ; $8AA3B9
    trb.w $001A                 ; $8AA3BC
    bne CODE_8AA3F4             ; $8AA3BF
    rts                         ; $8AA3C1
CODE_8AA3C2:
    lda.b $4A                   ; $8AA3C2
    pha                         ; $8AA3C4
    phy                         ; $8AA3C5
    ldy.w #$0000                ; $8AA3C6
    dec.b $4A                   ; $8AA3C9
    dec.b $4A                   ; $8AA3CB
    lda.b ($4A),y               ; $8AA3CD
    cmp.w #$FFFF                ; $8AA3CF
    beq CODE_8AA3EF             ; $8AA3D2
    lda.w $1516,x               ; $8AA3D4
    and.w #$FFBE                ; $8AA3D7
    sta.w $1516,x               ; $8AA3DA
    lda.w $1514,x               ; $8AA3DD
    ora.w #$0041                ; $8AA3E0
    sta.w $1514,x               ; $8AA3E3
    stz.b $64                   ; $8AA3E6
    stz.b $66                   ; $8AA3E8
    stz.b $6E                   ; $8AA3EA
    jsr.w func_8AA48D           ; $8AA3EC
CODE_8AA3EF:
    ply                         ; $8AA3EF
    pla                         ; $8AA3F0
    sta.b $4A                   ; $8AA3F1
    rts                         ; $8AA3F3
CODE_8AA3F4:
    phy                         ; $8AA3F4
    ldy.w #$0008                ; $8AA3F5
    lda.b ($4A),y               ; $8AA3F8
    beq CODE_8AA417             ; $8AA3FA
    lda.w $1516,x               ; $8AA3FC
    and.w #$FFBE                ; $8AA3FF
    sta.w $1516,x               ; $8AA402
    lda.w $1518,x               ; $8AA405
    ora.w #$0041                ; $8AA408
    sta.w $1518,x               ; $8AA40B
    stz.b $64                   ; $8AA40E
    stz.b $66                   ; $8AA410
    stz.b $6E                   ; $8AA412
    jsr.w func_8AA48D           ; $8AA414
CODE_8AA417:
    ply                         ; $8AA417
    rts                         ; $8AA418
menu_select:
    lda.w $0D90                 ; $8AA419
    cmp.w $0D68                 ; $8AA41C
    beq CODE_8AA423             ; $8AA41F
    clc                         ; $8AA421
    rts                         ; $8AA422
CODE_8AA423:
    lda.w #$C0C0                ; $8AA423
    trb.w $000E                 ; $8AA426
    bne CODE_8AA445             ; $8AA429
    lda.w #$C0C0                ; $8AA42B
    trb.w $0012                 ; $8AA42E
    bne CODE_8AA445             ; $8AA431
    lda.w #$C0C0                ; $8AA433
    trb.w $0016                 ; $8AA436
    bne CODE_8AA445             ; $8AA439
    lda.w #$C0C0                ; $8AA43B
    trb.w $001A                 ; $8AA43E
    bne CODE_8AA445             ; $8AA441
    clc                         ; $8AA443
    rts                         ; $8AA444
CODE_8AA445:
    lda.b $62                   ; $8AA445
    pha                         ; $8AA447
    lda.b $4A                   ; $8AA448
    pha                         ; $8AA44A
    phx                         ; $8AA44B
    phy                         ; $8AA44C
    stz.b $5E                   ; $8AA44D
    ldy.w #$0002                ; $8AA44F
    lda.b ($4A),y               ; $8AA452
    sta.b $5C                   ; $8AA454
    jsr.w func_8AA49E           ; $8AA456
    jsr.w $A466                 ; $8AA459
    ply                         ; $8AA45C
    plx                         ; $8AA45D
    pla                         ; $8AA45E
    sta.b $4A                   ; $8AA45F
    pla                         ; $8AA461
menu_select_restore:
    sta.b $62                   ; $8AA462
    sec                         ; $8AA464
    rts                         ; $8AA465
    jmp.w ($015C)               ; $8AA466
func_8AA469:
    lda.b $3C                   ; $8AA469
    asl a                       ; $8AA46B
    asl a                       ; $8AA46C
    asl a                       ; $8AA46D
    sec                         ; $8AA46E
    sbc.w #$001C                ; $8AA46F
    sta.w $0D90                 ; $8AA472
    lda.b $5A                   ; $8AA475
    asl a                       ; $8AA477
    asl a                       ; $8AA478
    asl a                       ; $8AA479
    sta.w $0D78                 ; $8AA47A
    rts                         ; $8AA47D
clear_object_slots:
    phx                         ; $8AA47E
    stz.b $8C                   ; $8AA47F
    ldx.w #$00F6                ; $8AA481
CODE_8AA484:
    stz.w $0500,x               ; $8AA484
    dex                         ; $8AA487
    dex                         ; $8AA488
    bpl CODE_8AA484             ; $8AA489
    plx                         ; $8AA48B
    rts                         ; $8AA48C
func_8AA48D:
    pha                         ; $8AA48D
    sep.b #$20                  ; $8AA48E
    lda.b #$11                  ; $8AA490
    sta.b $FB                   ; $8AA492
    rep.b #$20                  ; $8AA494
    lda.w #$0A8C                ; $8AA496
    sta.w $0D7E                 ; $8AA499
    pla                         ; $8AA49C
    rts                         ; $8AA49D
func_8AA49E:
    pha                         ; $8AA49E
    sep.b #$20                  ; $8AA49F
    lda.b #$13                  ; $8AA4A1
    sta.b $FB                   ; $8AA4A3
    rep.b #$20                  ; $8AA4A5
    lda.w #$0A8C                ; $8AA4A7
    sta.w $0D7E                 ; $8AA4AA
    pla                         ; $8AA4AD
    rts                         ; $8AA4AE
func_8AA4AF:
    sep.b #$20                  ; $8AA4AF static
    lda.b #$12                  ; $8AA4B1 static
    sta.b $FB                   ; $8AA4B3 static
    rep.b #$20                  ; $8AA4B5 static
    lda.w #$0A8C                ; $8AA4B7 static
    sta.w $0D7E                 ; $8AA4BA static
    rts                         ; $8AA4BD static
func_8AA4BE:
    sta.b $4A                   ; $8AA4BE
    ldy.w #$0000                ; $8AA4C0
CODE_8AA4C3:
    lda.b ($4A),y               ; $8AA4C3
    beq CODE_8AA4CE             ; $8AA4C5
    jsr.w func_8AB680           ; $8AA4C7
    iny                         ; $8AA4CA
    iny                         ; $8AA4CB
    bra CODE_8AA4C3             ; $8AA4CC
CODE_8AA4CE:
    rts                         ; $8AA4CE
string_list_nth:
    sta.b $44                   ; $8AA4CF
    phx                         ; $8AA4D1
    phy                         ; $8AA4D2
    lda.w $1516,x               ; $8AA4D3
    and.w #$003C                ; $8AA4D6
    lsr a                       ; $8AA4D9
    lsr a                       ; $8AA4DA
    tax                         ; $8AA4DB
    beq CODE_8AA4F6             ; $8AA4DC
    sep.b #$20                  ; $8AA4DE
    ldy.w #$0000                ; $8AA4E0
CODE_8AA4E3:
    lda.b ($44),y               ; $8AA4E3
    beq CODE_8AA4EF             ; $8AA4E5
    rep.b #$20                  ; $8AA4E7
    inc.b $44                   ; $8AA4E9
    sep.b #$20                  ; $8AA4EB
    bra CODE_8AA4E3             ; $8AA4ED
CODE_8AA4EF:
    inc.b $44                   ; $8AA4EF
    dex                         ; $8AA4F1
    bne CODE_8AA4E3             ; $8AA4F2
    rep.b #$20                  ; $8AA4F4
CODE_8AA4F6:
    lda.b $44                   ; $8AA4F6
    ply                         ; $8AA4F8
    plx                         ; $8AA4F9
    rts                         ; $8AA4FA
menu_action_submenu:
    lda.w #$CBD2                ; $8AA4FB
    jsr.w menu_submenu_open     ; $8AA4FE
    rts                         ; $8AA501
menu_action_vs:
    lda.w #$CCEB                ; $8AA502
    jsr.w menu_submenu_open     ; $8AA505
    rts                         ; $8AA508
vs_players_mask:
    ldx.w #$0004                ; $8AA509
    stz.b $42                   ; $8AA50C
    lda.b $56                   ; $8AA50E
CODE_8AA510:
    lsr a                       ; $8AA510
    bcc CODE_8AA531             ; $8AA511
    pha                         ; $8AA513
    lda.b $42                   ; $8AA514
    sec                         ; $8AA516
    ror a                       ; $8AA517
    sta.b $42                   ; $8AA518
    pla                         ; $8AA51A
    dex                         ; $8AA51B
    dey                         ; $8AA51C
    bne CODE_8AA510             ; $8AA51D
    lda.b $42                   ; $8AA51F
CODE_8AA521:
    cpx.w #$0000                ; $8AA521
    beq CODE_8AA52A             ; $8AA524
    lsr a                       ; $8AA526
    dex                         ; $8AA527
    bra CODE_8AA521             ; $8AA528
CODE_8AA52A:
    ldy.w #$0008                ; $8AA52A
    jsr.w lsr_n                 ; $8AA52D
    rts                         ; $8AA530
CODE_8AA531:
    pha                         ; $8AA531 static
    lda.b $42                   ; $8AA532 static
    clc                         ; $8AA534 static
    ror a                       ; $8AA535 static
    sta.b $42                   ; $8AA536 static
    pla                         ; $8AA538 static
    dex                         ; $8AA539 static
    bra CODE_8AA510             ; $8AA53A static
menu_action_players:
    ldy.w #$0001                ; $8AA53C
    jsr.w vs_players_set        ; $8AA53F
    rts                         ; $8AA542
    incbin !baserom:$52543..$52558
vs_players_set:
    lda.l $8103DC               ; $8AA558
    and.w #$FFF8                ; $8AA55C
    sta.l $8103DC               ; $8AA55F
    jsr.w vs_players_mask       ; $8AA563
    ora.w #$0002                ; $8AA566
    sta.l $8103DC               ; $8AA569
    jsr.w $A573                 ; $8AA56D
    inc.b $6C                   ; $8AA570
    rts                         ; $8AA572
    jsr.w fade_out_wait         ; $8AA573
vs_screen_blank:
    stz.w $420C                 ; $8AA576
    stz.b $8E                   ; $8AA579
    lda.w #$0001                ; $8AA57B
    jsl.l wait_frames_long      ; $8AA57E
vs_screen_setup:
    jsr.w vs_palette_load       ; $8AA582
    jsr.w vs_graphics_load      ; $8AA585
    sep.b #$20                  ; $8AA588
    lda.b #$00                  ; $8AA58A
    sta.w $2121                 ; $8AA58C
    lda.w $C4F4                 ; $8AA58F
    sta.w $2122                 ; $8AA592
    lda.w $C4F5                 ; $8AA595
    sta.w $2122                 ; $8AA598
    lda.w $A766                 ; $8AA59B
    sta.w $210E                 ; $8AA59E
    lda.w $A767                 ; $8AA5A1
    sta.w $210E                 ; $8AA5A4
    lda.w $A764                 ; $8AA5A7
    sta.w $210D                 ; $8AA5AA
    lda.w $A765                 ; $8AA5AD
    sta.w $210D                 ; $8AA5B0
    lda.b #$01                  ; $8AA5B3
    sta.w $2105                 ; $8AA5B5
    rep.b #$20                  ; $8AA5B8
    lda.w #$0002                ; $8AA5BA
    sta.b $4C                   ; $8AA5BD
    jsr.w clear_tilemap_7f      ; $8AA5BF
    lda.w #$007C                ; $8AA5C2
    sta.w $0D60                 ; $8AA5C5
    sta.w $0D62                 ; $8AA5C8
    sta.w $0D64                 ; $8AA5CB
    sta.w $0D66                 ; $8AA5CE
    lda.w #$0002                ; $8AA5D1
    sta.w $0D78                 ; $8AA5D4
    sta.w $0D7A                 ; $8AA5D7
    sta.w $0D7C                 ; $8AA5DA
    sta.w $0D7E                 ; $8AA5DD
    sta.w $0D70                 ; $8AA5E0
    sta.w $0D72                 ; $8AA5E3
    sta.w $0D74                 ; $8AA5E6
    sta.w $0D76                 ; $8AA5E9
    lda.w #$0002                ; $8AA5EC
    sta.b $7A                   ; $8AA5EF
    sta.b $7C                   ; $8AA5F1
    lda.w #$0001                ; $8AA5F3
    sta.b $7E                   ; $8AA5F6
    ldx.w #$0013                ; $8AA5F8
    ldy.w #$0006                ; $8AA5FB
    stx.w $0DBC                 ; $8AA5FE
    sty.w $0DC4                 ; $8AA601
    ldx.w #$0093                ; $8AA604
    ldy.w #$0006                ; $8AA607
    stx.w $0DBE                 ; $8AA60A
    sty.w $0DC6                 ; $8AA60D
    ldx.w #$0013                ; $8AA610
    ldy.w #$0076                ; $8AA613
    stx.w $0DC0                 ; $8AA616
    sty.w $0DC8                 ; $8AA619
    ldx.w #$0093                ; $8AA61C
    ldy.w #$0076                ; $8AA61F
    stx.w $0DC2                 ; $8AA622
    sty.w $0DCA                 ; $8AA625
    ldx.w #$0002                ; $8AA628
    ldy.w #$0001                ; $8AA62B
    jsr.w vs_grid_draw          ; $8AA62E
    ldx.w #$0012                ; $8AA631
    ldy.w #$0001                ; $8AA634
    jsr.w vs_grid_draw          ; $8AA637
    ldx.w #$0002                ; $8AA63A
    ldy.w #$000F                ; $8AA63D
    jsr.w vs_grid_draw          ; $8AA640
    ldx.w #$0012                ; $8AA643
    ldy.w #$000F                ; $8AA646
    jsr.w vs_grid_draw          ; $8AA649
    lda.w #$0004                ; $8AA64C
    sta.w $0DAC                 ; $8AA64F
    sta.w $0DAE                 ; $8AA652
    sta.w $0DB0                 ; $8AA655
    sta.w $0DB2                 ; $8AA658
    lda.w #$0004                ; $8AA65B
    sta.w $0DB4                 ; $8AA65E
    sta.w $0DB6                 ; $8AA661
    sta.w $0DB8                 ; $8AA664
    sta.w $0DBA                 ; $8AA667
    lda.w #$0013                ; $8AA66A
    sta.w $212C                 ; $8AA66D
    jsr.w vs_names_load         ; $8AA670
    jsr.w vs_players_in         ; $8AA673
    inc.b $8E                   ; $8AA676
    jsr.w fade_in_start         ; $8AA678
CODE_8AA67B:
    lda.w #$0001                ; $8AA67B
    jsl.l wait_frames_long      ; $8AA67E
vs_screen_frame:
    lda.w $0DF4                 ; $8AA682
    beq CODE_8AA68D             ; $8AA685
    lda.w $000E                 ; $8AA687
    sta.w $0DCC                 ; $8AA68A
CODE_8AA68D:
    lda.w $0DF6                 ; $8AA68D
    beq CODE_8AA698             ; $8AA690
    lda.w $0012                 ; $8AA692 static
    sta.w $0DCE                 ; $8AA695 static
CODE_8AA698:
    lda.w $0DF8                 ; $8AA698
    beq CODE_8AA6A3             ; $8AA69B
    lda.w $0016                 ; $8AA69D static
    sta.w $0DD0                 ; $8AA6A0 static
CODE_8AA6A3:
    lda.w $0DFA                 ; $8AA6A3
    beq CODE_8AA6AE             ; $8AA6A6
    lda.w $001A                 ; $8AA6A8 static
    sta.w $0DD2                 ; $8AA6AB static
CODE_8AA6AE:
    jsr.w clear_object_slots    ; $8AA6AE
    ldx.w #$0000                ; $8AA6B1
    jsr.w vs_panel_frame        ; $8AA6B4
    ldx.w #$0002                ; $8AA6B7
    jsr.w vs_panel_frame        ; $8AA6BA
    ldx.w #$0004                ; $8AA6BD
    jsr.w vs_panel_frame        ; $8AA6C0
    ldx.w #$0006                ; $8AA6C3
    jsr.w vs_panel_frame        ; $8AA6C6
    lda.w $0DCC                 ; $8AA6C9
    sta.w $000E                 ; $8AA6CC
    lda.w $0DCE                 ; $8AA6CF
    sta.w $0012                 ; $8AA6D2
    lda.w $0DD0                 ; $8AA6D5
    sta.w $0016                 ; $8AA6D8
    lda.w $0DD2                 ; $8AA6DB
    sta.w $001A                 ; $8AA6DE
    lda.w $019A                 ; $8AA6E1
    bne CODE_8AA67B             ; $8AA6E4
    jsr.w vs_names_absent       ; $8AA6E6
    lda.w #$0013                ; $8AA6E9
    sta.w $212C                 ; $8AA6EC
    lda.w #$0082                ; $8AA6EF
    sta.w $420C                 ; $8AA6F2
    rts                         ; $8AA6F5
vs_names_absent:
    ldx.w #$0006                ; $8AA6F6
CODE_8AA6F9:
    lda.w $0DF4,x               ; $8AA6F9
    beq CODE_8AA703             ; $8AA6FC
CODE_8AA6FE:
    dex                         ; $8AA6FE
    dex                         ; $8AA6FF
    bpl CODE_8AA6F9             ; $8AA700
    rts                         ; $8AA702
CODE_8AA703:
    lda.w $A804,x               ; $8AA703
    sta.b $42                   ; $8AA706
    lda.w $A80C,x               ; $8AA708
    sta.b $44                   ; $8AA70B
    sep.b #$20                  ; $8AA70D
    ldy.w #$0000                ; $8AA70F
CODE_8AA712:
    lda.b ($44),y               ; $8AA712
    sta.b ($42),y               ; $8AA714
    cmp.b #$00                  ; $8AA716
    beq CODE_8AA71D             ; $8AA718
    iny                         ; $8AA71A
    bra CODE_8AA712             ; $8AA71B
CODE_8AA71D:
    rep.b #$20                  ; $8AA71D
    bra CODE_8AA6FE             ; $8AA71F
vs_panel_screens_bg1_off:
    pha                         ; $8AA721 static
    sep.b #$20                  ; $8AA722 static
    lda.b #$12                  ; $8AA724 static
    bra CODE_8AA734             ; $8AA726 static
vs_panel_screens_bg2_off:
    pha                         ; $8AA728
    sep.b #$20                  ; $8AA729
    lda.b #$11                  ; $8AA72B
    bra CODE_8AA734             ; $8AA72D
vs_panel_screens:
    pha                         ; $8AA72F
    sep.b #$20                  ; $8AA730
    lda.b #$13                  ; $8AA732
CODE_8AA734:
    cpx.w #$0000                ; $8AA734
    beq CODE_8AA74C             ; $8AA737
    cpx.w #$0002                ; $8AA739
    beq CODE_8AA752             ; $8AA73C
    cpx.w #$0004                ; $8AA73E
    beq CODE_8AA758             ; $8AA741
    cpx.w #$0006                ; $8AA743
    beq CODE_8AA75E             ; $8AA746
    rep.b #$20                  ; $8AA748 static
    pla                         ; $8AA74A static
    rts                         ; $8AA74B static
CODE_8AA74C:
    sta.b $FB                   ; $8AA74C
    rep.b #$20                  ; $8AA74E
    pla                         ; $8AA750
    rts                         ; $8AA751
CODE_8AA752:
    sta.b $FC                   ; $8AA752
    rep.b #$20                  ; $8AA754
    pla                         ; $8AA756
    rts                         ; $8AA757
CODE_8AA758:
    sta.b $FB                   ; $8AA758
    rep.b #$20                  ; $8AA75A
    pla                         ; $8AA75C
    rts                         ; $8AA75D
CODE_8AA75E:
    sta.b $FC                   ; $8AA75E
    rep.b #$20                  ; $8AA760
    pla                         ; $8AA762
    rts                         ; $8AA763
    incbin !baserom:$52764..$52768
vs_ready_check:
    jsr.w vs_all_ready          ; $8AA768
    lda.w $0DDC                 ; $8AA76B
    cmp.w #$FFFD                ; $8AA76E
    bne CODE_8AA794             ; $8AA771
    lda.w $0DDE                 ; $8AA773
    cmp.w #$FFFD                ; $8AA776
    bne CODE_8AA794             ; $8AA779
    lda.w $0DE0                 ; $8AA77B
    cmp.w #$FFFD                ; $8AA77E
    bne CODE_8AA794             ; $8AA781
    lda.w $0DE2                 ; $8AA783
    cmp.w #$FFFD                ; $8AA786
    bne CODE_8AA794             ; $8AA789
    stz.w $0198                 ; $8AA78B
    lda.w #$FF80                ; $8AA78E
    sta.w $0196                 ; $8AA791
CODE_8AA794:
    rts                         ; $8AA794
vs_all_ready:
    phx                         ; $8AA795
    stz.b $42                   ; $8AA796
    stz.b $44                   ; $8AA798
    ldx.w #$0000                ; $8AA79A
CODE_8AA79D:
    lda.w $0DF4,x               ; $8AA79D
    cmp.w #$FFFF                ; $8AA7A0
    beq CODE_8AA7A7             ; $8AA7A3
    bra CODE_8AA7B5             ; $8AA7A5
CODE_8AA7A7:
    inc.b $42                   ; $8AA7A7
    lda.w $0DDC,x               ; $8AA7A9
    cmp.w #$FFFD                ; $8AA7AC
    beq CODE_8AA7B3             ; $8AA7AF
    bra CODE_8AA7B5             ; $8AA7B1
CODE_8AA7B3:
    inc.b $44                   ; $8AA7B3
CODE_8AA7B5:
    inx                         ; $8AA7B5
    inx                         ; $8AA7B6
    cpx.w #$0008                ; $8AA7B7
    bne CODE_8AA79D             ; $8AA7BA
    lda.b $42                   ; $8AA7BC
    cmp.b $44                   ; $8AA7BE
    bne CODE_8AA7CB             ; $8AA7C0
    stz.w $0198                 ; $8AA7C2
    lda.w #$FFC0                ; $8AA7C5
    sta.w $0196                 ; $8AA7C8
CODE_8AA7CB:
    plx                         ; $8AA7CB
    rts                         ; $8AA7CC
vs_palette_load:
    lda.w #$8A00                ; $8AA7CD
    ldx.w #$C5D8                ; $8AA7D0
    ldy.w #$0080                ; $8AA7D3
    jsl.l upload_cgram_now      ; $8AA7D6
    rts                         ; $8AA7DA
vs_players_in:
    lda.l $8103DC               ; $8AA7DB
    lsr a                       ; $8AA7DF
    lsr a                       ; $8AA7E0
    lsr a                       ; $8AA7E1
    lsr a                       ; $8AA7E2
    ldx.w #$0000                ; $8AA7E3
CODE_8AA7E6:
    lsr a                       ; $8AA7E6
    bcs CODE_8AA7F7             ; $8AA7E7
    stz.w $0DDC,x               ; $8AA7E9
    stz.w $0DF4,x               ; $8AA7EC
CODE_8AA7EF:
    inx                         ; $8AA7EF
    inx                         ; $8AA7F0
    cpx.w #$0008                ; $8AA7F1
    bne CODE_8AA7E6             ; $8AA7F4
    rts                         ; $8AA7F6
CODE_8AA7F7:
    pha                         ; $8AA7F7
    lda.w #$FFFF                ; $8AA7F8
    sta.w $0DDC,x               ; $8AA7FB
    sta.w $0DF4,x               ; $8AA7FE
    pla                         ; $8AA801
    bra CODE_8AA7EF             ; $8AA802
    incbin !baserom:$52804..$5281C
vs_panel_frame:
    lda.w $0DDC,x               ; $8AA81C
    cmp.w #$FFFE                ; $8AA81F
    beq CODE_8AA842             ; $8AA822
    cmp.w #$FFFD                ; $8AA824
    beq vs_ready_draw           ; $8AA827
    jsr.w vs_panel_name_draw    ; $8AA829
    jsr.w vs_name_entry_frame   ; $8AA82C
    jsr.w vs_letter_index       ; $8AA82F
    jsr.w vs_letter_width       ; $8AA832
    jsr.w vs_cursor_target      ; $8AA835
    jsr.w sprite_frame          ; $8AA838
    jsr.w vs_name_key           ; $8AA83B
    jsr.w vs_caret_blink        ; $8AA83E
    rts                         ; $8AA841
CODE_8AA842:
    jsr.w vs_cursor_step        ; $8AA842
    jsr.w vs_cursor_column      ; $8AA845
    jsr.w vs_car_info           ; $8AA848
    jsr.w vs_car_confirm        ; $8AA84B
    jsr.w sprite_frame          ; $8AA84E
    rts                         ; $8AA851
vs_car_confirm:
    jsr.w vs_cursor_moving      ; $8AA852
    bcc CODE_8AA858             ; $8AA855
    rts                         ; $8AA857
CODE_8AA858:
    lda.w $0D38,x               ; $8AA858
    cmp.w #$FFFF                ; $8AA85B
    bne CODE_8AA8A1             ; $8AA85E
    lda.w $0DCC,x               ; $8AA860
    and.w #$C0C0                ; $8AA863
    bne CODE_8AA8A1             ; $8AA866
    rts                         ; $8AA868
vs_ready_draw:
    lda.w $AA92,x               ; $8AA869
    sta.b $3A                   ; $8AA86C
    lda.w $AA9A,x               ; $8AA86E
    clc                         ; $8AA871
    adc.w #$0004                ; $8AA872
    sta.b $3C                   ; $8AA875
    lda.b $66                   ; $8AA877
    and.w #$0001                ; $8AA879
    bne CODE_8AA882             ; $8AA87C
    jsr.w vs_panel_text_clear   ; $8AA87E
    rts                         ; $8AA881
CODE_8AA882:
    txy                         ; $8AA882
    lda.b $3A                   ; $8AA883
    pha                         ; $8AA885
    lda.w $A804,y               ; $8AA886
    jsr.w vs_text_center        ; $8AA889
    jsr.w vs_put_string         ; $8AA88C
    pla                         ; $8AA88F
    clc                         ; $8AA890
    adc.w #$0004                ; $8AA891
    sta.b $3A                   ; $8AA894
    inc.b $3C                   ; $8AA896
    inc.b $3C                   ; $8AA898
    lda.w #$A9A1                ; $8AA89A
    jsr.w vs_put_string         ; $8AA89D
    rts                         ; $8AA8A0
CODE_8AA8A1:
    jsr.w vs_panel_text_clear   ; $8AA8A1
    jsr.w vs_name_default       ; $8AA8A4
    lda.w #$FFFD                ; $8AA8A7
    sta.w $0DDC,x               ; $8AA8AA
    jsr.w vs_panel_screens      ; $8AA8AD
    lda.w $0DB4,x               ; $8AA8B0
    sep.b #$20                  ; $8AA8B3
    cpx.w #$0000                ; $8AA8B5
    beq CODE_8AA8C9             ; $8AA8B8
    cpx.w #$0002                ; $8AA8BA
    beq CODE_8AA8CF             ; $8AA8BD
    cpx.w #$0004                ; $8AA8BF
    beq CODE_8AA8D5             ; $8AA8C2
    cpx.w #$0006                ; $8AA8C4
    beq CODE_8AA8DB             ; $8AA8C7
CODE_8AA8C9:
    sta.l $810418               ; $8AA8C9
    bra CODE_8AA8DF             ; $8AA8CD
CODE_8AA8CF:
    sta.l $810419               ; $8AA8CF
    bra CODE_8AA8DF             ; $8AA8D3
CODE_8AA8D5:
    sta.l $81041A               ; $8AA8D5
    bra CODE_8AA8DF             ; $8AA8D9
CODE_8AA8DB:
    sta.l $81041B               ; $8AA8DB
CODE_8AA8DF:
    rep.b #$20                  ; $8AA8DF
    jsr.w vs_ready_check        ; $8AA8E1
    rts                         ; $8AA8E4
vs_name_default:
    stz.b $46                   ; $8AA8E5
    lda.w $A804,x               ; $8AA8E7
    sta.b $42                   ; $8AA8EA
    ldy.w #$0000                ; $8AA8EC
CODE_8AA8EF:
    lda.b ($42),y               ; $8AA8EF
    and.w #$00FF                ; $8AA8F1
    beq CODE_8AA905             ; $8AA8F4
    cmp.w #$0041                ; $8AA8F6
    bmi CODE_8AA902             ; $8AA8F9
    cmp.w #$005B                ; $8AA8FB
    bpl CODE_8AA902             ; $8AA8FE
    inc.b $46                   ; $8AA900
CODE_8AA902:
    iny                         ; $8AA902
    bra CODE_8AA8EF             ; $8AA903
CODE_8AA905:
    lda.b $46                   ; $8AA905
    beq CODE_8AA90A             ; $8AA907
    rts                         ; $8AA909
CODE_8AA90A:
    ldy.w #$0000                ; $8AA90A static
    lda.w $A814,x               ; $8AA90D static
    sta.b $44                   ; $8AA910 static
    sep.b #$20                  ; $8AA912 static
CODE_8AA914:
    lda.b ($44),y               ; $8AA914 static
    sta.b ($42),y               ; $8AA916 static
    cmp.b #$00                  ; $8AA918 static
    beq CODE_8AA91F             ; $8AA91A static
    iny                         ; $8AA91C static
    bra CODE_8AA914             ; $8AA91D static
CODE_8AA91F:
    rep.b #$20                  ; $8AA91F static
    rts                         ; $8AA921 static
vs_names_load:
    sep.b #$20                  ; $8AA922
    ldx.w #$0000                ; $8AA924
CODE_8AA927:
    lda.w $CD7C,x               ; $8AA927
    beq CODE_8AA933             ; $8AA92A
    sta.l $8103E4,x             ; $8AA92C
    inx                         ; $8AA930
    bra CODE_8AA927             ; $8AA931
CODE_8AA933:
    sta.l $8103E4,x             ; $8AA933
    stx.w $0DD4                 ; $8AA937
    ldx.w #$0000                ; $8AA93A
CODE_8AA93D:
    lda.w $CD87,x               ; $8AA93D
    beq CODE_8AA949             ; $8AA940
    sta.l $8103F1,x             ; $8AA942
    inx                         ; $8AA946
    bra CODE_8AA93D             ; $8AA947
CODE_8AA949:
    sta.l $8103F1,x             ; $8AA949
    stx.w $0DD6                 ; $8AA94D
    ldx.w #$0000                ; $8AA950
CODE_8AA953:
    lda.w $CD92,x               ; $8AA953
    beq CODE_8AA95F             ; $8AA956
    sta.l $8103FE,x             ; $8AA958
    inx                         ; $8AA95C
    bra CODE_8AA953             ; $8AA95D
CODE_8AA95F:
    sta.l $8103FE,x             ; $8AA95F
    stx.w $0DD8                 ; $8AA963
    ldx.w #$0000                ; $8AA966
CODE_8AA969:
    lda.w $CD9F,x               ; $8AA969
    beq CODE_8AA975             ; $8AA96C
    sta.l $81040B,x             ; $8AA96E
    inx                         ; $8AA972
    bra CODE_8AA969             ; $8AA973
CODE_8AA975:
    sta.l $81040B,x             ; $8AA975
    stx.w $0DDA                 ; $8AA979
    rep.b #$20                  ; $8AA97C
    rts                         ; $8AA97E
vs_text_center:
    pha                         ; $8AA97F
    sta.b $48                   ; $8AA980
    ldy.w #$0000                ; $8AA982
    sep.b #$20                  ; $8AA985
CODE_8AA987:
    lda.b ($48),y               ; $8AA987
    beq CODE_8AA98E             ; $8AA989
    iny                         ; $8AA98B
    bra CODE_8AA987             ; $8AA98C
CODE_8AA98E:
    rep.b #$20                  ; $8AA98E
    tya                         ; $8AA990
    lsr a                       ; $8AA991
    sta.b $48                   ; $8AA992
    lda.w #$0006                ; $8AA994
    sec                         ; $8AA997
    sbc.b $48                   ; $8AA998
    clc                         ; $8AA99A
    adc.b $3A                   ; $8AA99B
    sta.b $3A                   ; $8AA99D
    pla                         ; $8AA99F
    rts                         ; $8AA9A0
    incbin !baserom:$529A1..$529A7
vs_car_info:
    lda.w $0DB4,x               ; $8AA9A7
    cmp.w $0DAC,x               ; $8AA9AA
    beq CODE_8AA9F1             ; $8AA9AD
    sta.w $0DAC,x               ; $8AA9AF
    asl a                       ; $8AA9B2
    tay                         ; $8AA9B3
    lda.w $CE63,y               ; $8AA9B4
    sta.b $42                   ; $8AA9B7
    lda.w $AA92,x               ; $8AA9B9
    pha                         ; $8AA9BC
    lda.w $AA9A,x               ; $8AA9BD
    pha                         ; $8AA9C0
    jsr.w vs_panel_text_clear   ; $8AA9C1
    jsr.w vs_car_title          ; $8AA9C4
    pla                         ; $8AA9C7
    sta.b $3C                   ; $8AA9C8
    pla                         ; $8AA9CA
    sta.b $3A                   ; $8AA9CB
    inc.b $3C                   ; $8AA9CD
    ldy.w #$0000                ; $8AA9CF
CODE_8AA9D2:
    sep.b #$20                  ; $8AA9D2
    lda.b ($42),y               ; $8AA9D4
    beq CODE_8AA9E5             ; $8AA9D6
    cmp.b #$FF                  ; $8AA9D8
    beq CODE_8AA9E2             ; $8AA9DA
    jsr.w vs_put_char           ; $8AA9DC
    iny                         ; $8AA9DF
    bra CODE_8AA9D2             ; $8AA9E0
CODE_8AA9E2:
    rep.b #$20                  ; $8AA9E2
    rts                         ; $8AA9E4
CODE_8AA9E5:
    rep.b #$20                  ; $8AA9E5
    lda.w $AA92,x               ; $8AA9E7
    sta.b $3A                   ; $8AA9EA
    inc.b $3C                   ; $8AA9EC
    iny                         ; $8AA9EE
    bra CODE_8AA9D2             ; $8AA9EF
CODE_8AA9F1:
    rts                         ; $8AA9F1
vs_panel_reset:
    stz.w $0DB4,x               ; $8AA9F2
    lda.w $0D38,x               ; $8AA9F5
    cmp.w #$FFFF                ; $8AA9F8
    beq CODE_8AAA0A             ; $8AA9FB
    jsr.w rng_next_8a_2         ; $8AA9FD
    and.w #$0003                ; $8AAA00
    sta.w $0DB4,x               ; $8AAA03
    jsr.w vs_cursor_home        ; $8AAA06
    rts                         ; $8AAA09
CODE_8AAA0A:
    jsr.w vs_cursor_home        ; $8AAA0A
    rts                         ; $8AAA0D
vs_cursor_column:
    lda.w $0DB4,x               ; $8AAA0E
    asl a                       ; $8AAA11
    asl a                       ; $8AAA12
    asl a                       ; $8AAA13
    asl a                       ; $8AAA14
    clc                         ; $8AAA15
    adc.w $AA82,x               ; $8AAA16
    sta.w $0D88,x               ; $8AAA19
    lda.w $AA8A,x               ; $8AAA1C
    sta.w $0D90,x               ; $8AAA1F
    rts                         ; $8AAA22
vs_cursor_step:
    lda.w $0D38,x               ; $8AAA23
    cmp.w #$FFFF                ; $8AAA26
    beq CODE_8AAA2C             ; $8AAA29
    rts                         ; $8AAA2B
CODE_8AAA2C:
    jsr.w vs_cursor_moving      ; $8AAA2C
    bcc CODE_8AAA32             ; $8AAA2F
    rts                         ; $8AAA31
CODE_8AAA32:
    lda.w $0DCC,x               ; $8AAA32
    and.w #$0100                ; $8AAA35
    bne CODE_8AAA43             ; $8AAA38
    lda.w $0DCC,x               ; $8AAA3A
    and.w #$0200                ; $8AAA3D
    bne CODE_8AAA4F             ; $8AAA40
    rts                         ; $8AAA42
CODE_8AAA43:
    lda.w $0DB4,x               ; $8AAA43
    cmp.w #$0003                ; $8AAA46
    beq CODE_8AAA4E             ; $8AAA49
    inc.w $0DB4,x               ; $8AAA4B
CODE_8AAA4E:
    rts                         ; $8AAA4E
CODE_8AAA4F:
    lda.w $0DB4,x               ; $8AAA4F
    beq CODE_8AAA57             ; $8AAA52
    dec.w $0DB4,x               ; $8AAA54
CODE_8AAA57:
    rts                         ; $8AAA57
vs_cursor_home:
    lda.w $AA82,x               ; $8AAA58
    sta.w $0D88,x               ; $8AAA5B
    lda.w $AA8A,x               ; $8AAA5E
    sta.w $0D90,x               ; $8AAA61
    lda.w #$0004                ; $8AAA64
    sta.w $0D78,x               ; $8AAA67
    lda.w #$FFFF                ; $8AAA6A
    sta.w $0DAC,x               ; $8AAA6D
    rts                         ; $8AAA70
vs_car_title:
    lda.w $AA92,x               ; $8AAA71
    sta.b $3A                   ; $8AAA74
    lda.w $AA9A,x               ; $8AAA76
    sta.b $3C                   ; $8AAA79
    lda.w #$CE5B                ; $8AAA7B
    jsr.w vs_put_string         ; $8AAA7E
    rts                         ; $8AAA81
    incbin !baserom:$52A82..$52AA2
vs_caret_blink:
    lda.w $0DDC,x               ; $8AAAA2
    cmp.w #$FFFE                ; $8AAAA5
    bne CODE_8AAAAE             ; $8AAAA8
    stz.w $05F8,x               ; $8AAAAA
    rts                         ; $8AAAAD
CODE_8AAAAE:
    lda.b $66                   ; $8AAAAE
    and.w #$0001                ; $8AAAB0
    bne CODE_8AAABA             ; $8AAAB3
    lda.w #$0000                ; $8AAAB5
    bra CODE_8AAABD             ; $8AAAB8
CODE_8AAABA:
    lda.w #$FFFF                ; $8AAABA
CODE_8AAABD:
    sta.w $05F8,x               ; $8AAABD
    rts                         ; $8AAAC0
vs_panel_name_draw:
    phx                         ; $8AAAC1
    lda.w #$0001                ; $8AAAC2
    sta.b $88                   ; $8AAAC5
    cpx.w #$0000                ; $8AAAC7
    beq CODE_8AAADF             ; $8AAACA
    cpx.w #$0002                ; $8AAACC
    beq CODE_8AAB02             ; $8AAACF
    cpx.w #$0004                ; $8AAAD1
    beq CODE_8AAB25             ; $8AAAD4
    cpx.w #$0006                ; $8AAAD6
    beq CODE_8AAB48             ; $8AAAD9
    stz.b $88                   ; $8AAADB static
    plx                         ; $8AAADD static
    rts                         ; $8AAADE static
CODE_8AAADF:
    lda.w $ACDC                 ; $8AAADF
    asl a                       ; $8AAAE2
    asl a                       ; $8AAAE3
    asl a                       ; $8AAAE4
    sta.b $3C                   ; $8AAAE5
    lda.w $ACD4                 ; $8AAAE7
    asl a                       ; $8AAAEA
    asl a                       ; $8AAAEB
    asl a                       ; $8AAAEC
    sta.b $3A                   ; $8AAAED
    lda.w #$03E4                ; $8AAAEF
    jsr.w vs_put_string         ; $8AAAF2
    ldx.w #$0000                ; $8AAAF5
    jsr.w vs_panel_sprite       ; $8AAAF8
    stz.b $88                   ; $8AAAFB
    jsr.w vs_panel_marks        ; $8AAAFD
    plx                         ; $8AAB00
    rts                         ; $8AAB01
CODE_8AAB02:
    lda.w $ACDE                 ; $8AAB02
    asl a                       ; $8AAB05
    asl a                       ; $8AAB06
    asl a                       ; $8AAB07
    sta.b $3C                   ; $8AAB08
    lda.w $ACD6                 ; $8AAB0A
    asl a                       ; $8AAB0D
    asl a                       ; $8AAB0E
    asl a                       ; $8AAB0F
    sta.b $3A                   ; $8AAB10
    lda.w #$03F1                ; $8AAB12
    jsr.w vs_put_string         ; $8AAB15
    ldx.w #$0002                ; $8AAB18
    jsr.w vs_panel_sprite       ; $8AAB1B
    stz.b $88                   ; $8AAB1E
    jsr.w vs_panel_marks_1      ; $8AAB20
    plx                         ; $8AAB23
    rts                         ; $8AAB24
CODE_8AAB25:
    lda.w $ACE0                 ; $8AAB25
    asl a                       ; $8AAB28
    asl a                       ; $8AAB29
    asl a                       ; $8AAB2A
    sta.b $3C                   ; $8AAB2B
    lda.w $ACD8                 ; $8AAB2D
    asl a                       ; $8AAB30
    asl a                       ; $8AAB31
    asl a                       ; $8AAB32
    sta.b $3A                   ; $8AAB33
    lda.w #$03FE                ; $8AAB35
    jsr.w vs_put_string         ; $8AAB38
    ldx.w #$0004                ; $8AAB3B
    jsr.w vs_panel_sprite       ; $8AAB3E
    stz.b $88                   ; $8AAB41
    jsr.w vs_panel_marks_2      ; $8AAB43
    plx                         ; $8AAB46
    rts                         ; $8AAB47
CODE_8AAB48:
    lda.w $ACE2                 ; $8AAB48
    asl a                       ; $8AAB4B
    asl a                       ; $8AAB4C
    asl a                       ; $8AAB4D
    sta.b $3C                   ; $8AAB4E
    lda.w $ACDA                 ; $8AAB50
    asl a                       ; $8AAB53
    asl a                       ; $8AAB54
    asl a                       ; $8AAB55
    sta.b $3A                   ; $8AAB56
    lda.w #$040B                ; $8AAB58
    jsr.w vs_put_string         ; $8AAB5B
    ldx.w #$0006                ; $8AAB5E
    jsr.w vs_panel_sprite       ; $8AAB61
    stz.b $88                   ; $8AAB64
    jsr.w vs_panel_marks_3      ; $8AAB66
    plx                         ; $8AAB69
    rts                         ; $8AAB6A
vs_panel_marks:
    lda.w #$0049                ; $8AAB6B
    ldx.w #$0021                ; $8AAB6E
    ldy.w #$0018                ; $8AAB71
    jsr.w vs_put_sprite_char    ; $8AAB74
    lda.w #$0054                ; $8AAB77
    ldx.w #$0063                ; $8AAB7A
    ldy.w #$0048                ; $8AAB7D
    jsr.w vs_put_sprite_char    ; $8AAB80
    rts                         ; $8AAB83
vs_panel_marks_1:
    lda.w #$0049                ; $8AAB84
    ldx.w #$00A1                ; $8AAB87
    ldy.w #$0018                ; $8AAB8A
    jsr.w vs_put_sprite_char    ; $8AAB8D
    lda.w #$0054                ; $8AAB90
    ldx.w #$00E3                ; $8AAB93
    ldy.w #$0048                ; $8AAB96
    jsr.w vs_put_sprite_char    ; $8AAB99
    rts                         ; $8AAB9C
vs_panel_marks_2:
    lda.w #$0049                ; $8AAB9D
    ldx.w #$0021                ; $8AABA0
    ldy.w #$0088                ; $8AABA3
    jsr.w vs_put_sprite_char    ; $8AABA6
    lda.w #$0054                ; $8AABA9
    ldx.w #$0063                ; $8AABAC
    ldy.w #$00B8                ; $8AABAF
    jsr.w vs_put_sprite_char    ; $8AABB2
    rts                         ; $8AABB5
vs_panel_marks_3:
    lda.w #$0049                ; $8AABB6
    ldx.w #$00A1                ; $8AABB9
    ldy.w #$0088                ; $8AABBC
    jsr.w vs_put_sprite_char    ; $8AABBF
    lda.w #$0054                ; $8AABC2
    ldx.w #$00E3                ; $8AABC5
    ldy.w #$00B8                ; $8AABC8
    jsr.w vs_put_sprite_char    ; $8AABCB
    rts                         ; $8AABCE
vs_panel_sprite:
    lda.w $ACD4,x               ; $8AABCF
    asl a                       ; $8AABD2
    asl a                       ; $8AABD3
    asl a                       ; $8AABD4
    clc                         ; $8AABD5
    adc.b $8A                   ; $8AABD6
    sta.w $06F8,x               ; $8AABD8
    lda.w $ACDC,x               ; $8AABDB
    asl a                       ; $8AABDE
    asl a                       ; $8AABDF
    asl a                       ; $8AABE0
    clc                         ; $8AABE1
    adc.w #$0007                ; $8AABE2
    sta.w $07F8,x               ; $8AABE5
    lda.w #$3002                ; $8AABE8
    sta.w $08F8,x               ; $8AABEB
    rts                         ; $8AABEE
vs_name_key:
    lda.w $0DCC,x               ; $8AABEF
    and.w #$C0C0                ; $8AABF2
    bne CODE_8AABF8             ; $8AABF5
    rts                         ; $8AABF7
CODE_8AABF8:
    eor.w #$FFFF                ; $8AABF8
    sta.w $0DCC,x               ; $8AABFB
    ldy.b $4E                   ; $8AABFE
    lda.w $AFF1,y               ; $8AAC00
    and.w #$00FF                ; $8AAC03
    cmp.w #$0065                ; $8AAC06
    beq CODE_8AAC2E             ; $8AAC09
    cmp.w #$0064                ; $8AAC0B
    beq CODE_8AAC5D             ; $8AAC0E
    pha                         ; $8AAC10
    jsr.w vs_name_clear         ; $8AAC11
    lda.w $0DD4,x               ; $8AAC14
    cmp.w #$000C                ; $8AAC17
    beq CODE_8AAC29             ; $8AAC1A
    pla                         ; $8AAC1C
    cmp.w #$0073                ; $8AAC1D
    beq CODE_8AAC50             ; $8AAC20
    jsr.w vs_name_put           ; $8AAC22
    inc.w $0DD4,x               ; $8AAC25
    rts                         ; $8AAC28
CODE_8AAC29:
    jsr.w vs_panel_screens_bg1_off; $8AAC29 static
    pla                         ; $8AAC2C static
    rts                         ; $8AAC2D static
CODE_8AAC2E:
    lda.w #$0000                ; $8AAC2E
    jsr.w vs_name_put           ; $8AAC31
    jsr.w vs_cursor_moving      ; $8AAC34
    bcc CODE_8AAC3A             ; $8AAC37
    rts                         ; $8AAC39
CODE_8AAC3A:
    jsr.w vs_panel_text_clear   ; $8AAC3A
    lda.w $0DDC,x               ; $8AAC3D
    sta.w $0D38,x               ; $8AAC40
    lda.w #$FFFE                ; $8AAC43
    sta.w $0DDC,x               ; $8AAC46
    jsr.w vs_panel_reset        ; $8AAC49
    jsr.w vs_panel_screens      ; $8AAC4C
    rts                         ; $8AAC4F
CODE_8AAC50:
    jsr.w vs_name_clear         ; $8AAC50 static
    lda.w #$0020                ; $8AAC53 static
    jsr.w vs_name_put           ; $8AAC56 static
    inc.w $0DD4,x               ; $8AAC59 static
    rts                         ; $8AAC5C static
CODE_8AAC5D:
    jsr.w vs_name_clear         ; $8AAC5D static
    lda.w $0DD4,x               ; $8AAC60 static
    beq CODE_8AAC6F             ; $8AAC63 static
    dec.w $0DD4,x               ; $8AAC65 static
    lda.w #$0000                ; $8AAC68 static
    jsr.w vs_name_put           ; $8AAC6B static
    rts                         ; $8AAC6E static
CODE_8AAC6F:
    jsr.w vs_panel_screens_bg1_off; $8AAC6F static
    rts                         ; $8AAC72 static
vs_name_put:
    pha                         ; $8AAC73
    phx                         ; $8AAC74
    phy                         ; $8AAC75
    jsr.w vs_panel_screens_bg2_off; $8AAC76
    sep.b #$20                  ; $8AAC79
    ldy.w $0DD4,x               ; $8AAC7B
    cpx.w #$0000                ; $8AAC7E
    beq CODE_8AAC92             ; $8AAC81
    cpx.w #$0002                ; $8AAC83
    beq CODE_8AAC99             ; $8AAC86
    cpx.w #$0004                ; $8AAC88
    beq CODE_8AACA0             ; $8AAC8B
    cpx.w #$0006                ; $8AAC8D
    beq CODE_8AACA7             ; $8AAC90
CODE_8AAC92:
    tyx                         ; $8AAC92
    sta.l $8103E4,x             ; $8AAC93
    bra CODE_8AACAC             ; $8AAC97
CODE_8AAC99:
    tyx                         ; $8AAC99
    sta.l $8103F1,x             ; $8AAC9A
    bra CODE_8AACAC             ; $8AAC9E
CODE_8AACA0:
    tyx                         ; $8AACA0
    sta.l $8103FE,x             ; $8AACA1
    bra CODE_8AACAC             ; $8AACA5
CODE_8AACA7:
    tyx                         ; $8AACA7
    sta.l $81040B,x             ; $8AACA8
CODE_8AACAC:
    rep.b #$20                  ; $8AACAC
    ply                         ; $8AACAE
    plx                         ; $8AACAF
    pla                         ; $8AACB0
    rts                         ; $8AACB1
vs_name_clear:
    pha                         ; $8AACB2
    lda.w $0DEC,x               ; $8AACB3
    bne CODE_8AACD2             ; $8AACB6
    inc.w $0DEC,x               ; $8AACB8
    stz.w $0DD4,x               ; $8AACBB
    lda.w $A804,x               ; $8AACBE
    sta.b $48                   ; $8AACC1
    ldy.w #$0000                ; $8AACC3
    lda.w #$0000                ; $8AACC6
CODE_8AACC9:
    sta.b ($48),y               ; $8AACC9
    iny                         ; $8AACCB
    iny                         ; $8AACCC
    cpy.w #$000C                ; $8AACCD
    bne CODE_8AACC9             ; $8AACD0
CODE_8AACD2:
    pla                         ; $8AACD2
    rts                         ; $8AACD3
    incbin !baserom:$52CD4..$52CE4
vs_letter_index:
    lda.w $0DB4,x               ; $8AACE4
    tay                         ; $8AACE7
    lda.w #$0000                ; $8AACE8
    cpy.w #$0000                ; $8AACEB
    beq CODE_8AACF7             ; $8AACEE
    clc                         ; $8AACF0
CODE_8AACF1:
    adc.w #$0007                ; $8AACF1
    dey                         ; $8AACF4
    bne CODE_8AACF1             ; $8AACF5
CODE_8AACF7:
    clc                         ; $8AACF7
    adc.w $0DAC,x               ; $8AACF8
    sta.b $4E                   ; $8AACFB
    tay                         ; $8AACFD
    rts                         ; $8AACFE
vs_letter_width:
    ldy.b $4E                   ; $8AACFF
    lda.w $B014,y               ; $8AAD01
    and.w #$00FF                ; $8AAD04
    sta.w $0D78,x               ; $8AAD07
    lda.w $AFF1,y               ; $8AAD0A
    and.w #$00FF                ; $8AAD0D
    cmp.w #$0073                ; $8AAD10
    beq CODE_8AAD22             ; $8AAD13
    cmp.w #$0064                ; $8AAD15
    beq CODE_8AAD28             ; $8AAD18
    cmp.w #$0065                ; $8AAD1A
    beq CODE_8AAD2E             ; $8AAD1D
    stz.b $50                   ; $8AAD1F
    rts                         ; $8AAD21
CODE_8AAD22:
    lda.w #$0009                ; $8AAD22 static
    sta.b $50                   ; $8AAD25 static
    rts                         ; $8AAD27 static
CODE_8AAD28:
    lda.w #$0008                ; $8AAD28 static
    sta.b $50                   ; $8AAD2B static
    rts                         ; $8AAD2D static
CODE_8AAD2E:
    lda.w #$0009                ; $8AAD2E
    sta.b $50                   ; $8AAD31
    rts                         ; $8AAD33
vs_cursor_target:
    lda.w $0DAC,x               ; $8AAD34
    asl a                       ; $8AAD37
    asl a                       ; $8AAD38
    asl a                       ; $8AAD39
    asl a                       ; $8AAD3A
    clc                         ; $8AAD3B
    adc.w $0DBC,x               ; $8AAD3C
    sec                         ; $8AAD3F
    sbc.w #$0080                ; $8AAD40
    clc                         ; $8AAD43
    adc.b $50                   ; $8AAD44
    sta.w $0D88,x               ; $8AAD46
    lda.w $0DB4,x               ; $8AAD49
    asl a                       ; $8AAD4C
    asl a                       ; $8AAD4D
    asl a                       ; $8AAD4E
    asl a                       ; $8AAD4F
    clc                         ; $8AAD50
    adc.w $0DC4,x               ; $8AAD51
    sta.w $0D90,x               ; $8AAD54
    rts                         ; $8AAD57
vs_cursor_moving:
    lda.w $0D88,x               ; $8AAD58
    cmp.w $0D80,x               ; $8AAD5B
    bne CODE_8AAD72             ; $8AAD5E
    lda.w $0D78,x               ; $8AAD60
    cmp.w $0D70,x               ; $8AAD63
    bne CODE_8AAD72             ; $8AAD66
    lda.w $0D90,x               ; $8AAD68
    sbc.w $0D68,x               ; $8AAD6B
    bne CODE_8AAD72             ; $8AAD6E
    clc                         ; $8AAD70
    rts                         ; $8AAD71
CODE_8AAD72:
    sec                         ; $8AAD72
    rts                         ; $8AAD73
vs_name_entry_frame:
    lda.w $0DE4,x               ; $8AAD74
    beq CODE_8AAD7D             ; $8AAD77
    dec.w $0DE4,x               ; $8AAD79
    rts                         ; $8AAD7C
CODE_8AAD7D:
    lda.w $0DDC,x               ; $8AAD7D
    cmp.w #$FFFF                ; $8AAD80
    beq CODE_8AADD4             ; $8AAD83
    jsr.w vs_cursor_moving      ; $8AAD85
    bcc CODE_8AAD8B             ; $8AAD88
    rts                         ; $8AAD8A
CODE_8AAD8B:
    lda.w $0DCC,x               ; $8AAD8B
    ora.w #$C0C0                ; $8AAD8E
    sta.w $0DCC,x               ; $8AAD91
    jsr.w rng_next_8a_2         ; $8AAD94
    and.w #$0003                ; $8AAD97
    bne CODE_8AAD9F             ; $8AAD9A
    lda.w #$0002                ; $8AAD9C
CODE_8AAD9F:
    asl a                       ; $8AAD9F
    asl a                       ; $8AADA0
    asl a                       ; $8AADA1
    asl a                       ; $8AADA2
    sta.w $0DE4,x               ; $8AADA3
    lda.w $A80C,x               ; $8AADA6
    sta.b $54                   ; $8AADA9
    ldy.w $0DDC,x               ; $8AADAB
    lda.b ($54),y               ; $8AADAE
    and.w #$00FF                ; $8AADB0
    beq CODE_8AADBC             ; $8AADB3
    jsr.w vs_letter_find        ; $8AADB5
    inc.w $0DDC,x               ; $8AADB8
    rts                         ; $8AADBB
CODE_8AADBC:
    lda.w #$0004                ; $8AADBC
    sta.w $0DAC,x               ; $8AADBF
    sta.w $0DB4,x               ; $8AADC2
    lda.w #$0000                ; $8AADC5
    jsr.w vs_name_put           ; $8AADC8
    rts                         ; $8AADCB
    incbin !baserom:$52DCC..$52DD4
CODE_8AADD4:
    lda.w $0DCC,x               ; $8AADD4
    sta.b $42                   ; $8AADD7
    and.w #$0800                ; $8AADD9
    beq CODE_8AADE9             ; $8AADDC
    lda.w $0D90,x               ; $8AADDE static
    cmp.w $0D68,x               ; $8AADE1 static
    bne CODE_8AADE9             ; $8AADE4 static
    jsr.w vs_letter_prev        ; $8AADE6 static
CODE_8AADE9:
    lda.b $42                   ; $8AADE9
    and.w #$0400                ; $8AADEB
    beq CODE_8AADFB             ; $8AADEE
    lda.w $0D90,x               ; $8AADF0
    cmp.w $0D68,x               ; $8AADF3
    bne CODE_8AADFB             ; $8AADF6
    jsr.w vs_letter_next        ; $8AADF8
CODE_8AADFB:
    lda.b $42                   ; $8AADFB
    and.w #$0100                ; $8AADFD
    beq CODE_8AAE0D             ; $8AAE00
    lda.w $0D88,x               ; $8AAE02
    cmp.w $0D80,x               ; $8AAE05
    bne CODE_8AAE0D             ; $8AAE08
    jsr.w vs_row_next           ; $8AAE0A
CODE_8AAE0D:
    lda.b $42                   ; $8AAE0D
    and.w #$0200                ; $8AAE0F
    beq CODE_8AAE1F             ; $8AAE12
    lda.w $0D88,x               ; $8AAE14 static
    cmp.w $0D80,x               ; $8AAE17 static
    bne CODE_8AAE1F             ; $8AAE1A static
    jsr.w vs_row_prev           ; $8AAE1C static
CODE_8AAE1F:
    rts                         ; $8AAE1F
vs_letter_find:
    ldy.w #$0000                ; $8AAE20
    sep.b #$20                  ; $8AAE23
CODE_8AAE25:
    cmp.w $AF57,y               ; $8AAE25
    beq CODE_8AAE2F             ; $8AAE28
    iny                         ; $8AAE2A
    iny                         ; $8AAE2B
    iny                         ; $8AAE2C
    bra CODE_8AAE25             ; $8AAE2D
CODE_8AAE2F:
    iny                         ; $8AAE2F
    lda.w $AF57,y               ; $8AAE30
    sta.w $0DAC,x               ; $8AAE33
    stz.w $0DAD,x               ; $8AAE36
    iny                         ; $8AAE39
    lda.w $AF57,y               ; $8AAE3A
    sta.w $0DB4,x               ; $8AAE3D
    stz.w $0DB5,x               ; $8AAE40
    rep.b #$20                  ; $8AAE43
    rts                         ; $8AAE45
vs_panel_text_clear:
    phx                         ; $8AAE46
    lda.w #$007F                ; $8AAE47
    sta.b $72                   ; $8AAE4A
    jsr.w vs_panel_text_base    ; $8AAE4C
    ldx.w #$000E                ; $8AAE4F
CODE_8AAE52:
    ldy.w #$0000                ; $8AAE52
    lda.w #$003D                ; $8AAE55
CODE_8AAE58:
    sta.b [$70],y               ; $8AAE58
    iny                         ; $8AAE5A
    iny                         ; $8AAE5B
    cpy.w #$001E                ; $8AAE5C
    bne CODE_8AAE58             ; $8AAE5F
    lda.b $70                   ; $8AAE61
    clc                         ; $8AAE63
    adc.w #$0040                ; $8AAE64
    sta.b $70                   ; $8AAE67
    dex                         ; $8AAE69
    bne CODE_8AAE52             ; $8AAE6A
    plx                         ; $8AAE6C
    rts                         ; $8AAE6D
vs_panel_text_base:
    cpx.w #$0000                ; $8AAE6E
    beq CODE_8AAE83             ; $8AAE71
    cpx.w #$0002                ; $8AAE73
    beq CODE_8AAE89             ; $8AAE76
    cpx.w #$0004                ; $8AAE78
    beq CODE_8AAE8F             ; $8AAE7B
    cpx.w #$0006                ; $8AAE7D
    beq CODE_8AAE95             ; $8AAE80
    rts                         ; $8AAE82 static
CODE_8AAE83:
    lda.w #$0000                ; $8AAE83
    sta.b $70                   ; $8AAE86
    rts                         ; $8AAE88
CODE_8AAE89:
    lda.w #$0022                ; $8AAE89
    sta.b $70                   ; $8AAE8C
    rts                         ; $8AAE8E
CODE_8AAE8F:
    lda.w #$03C0                ; $8AAE8F
    sta.b $70                   ; $8AAE92
    rts                         ; $8AAE94
CODE_8AAE95:
    lda.w #$03E2                ; $8AAE95
    sta.b $70                   ; $8AAE98
    rts                         ; $8AAE9A
rng_next_8a_2:
    lda.b $12                   ; $8AAE9B
    bmi CODE_8AAEB2             ; $8AAE9D
    lda.b $10                   ; $8AAE9F
    asl a                       ; $8AAEA1
    eor.w #$2B45                ; $8AAEA2
    sta.b $10                   ; $8AAEA5
    lda.b $12                   ; $8AAEA7
    rol a                       ; $8AAEA9
    eor.w #$1D87                ; $8AAEAA
    sta.b $12                   ; $8AAEAD
    lda.b $10                   ; $8AAEAF
    rts                         ; $8AAEB1
CODE_8AAEB2:
    asl.b $10                   ; $8AAEB2
    rol.b $12                   ; $8AAEB4
    lda.b $10                   ; $8AAEB6
    rts                         ; $8AAEB8
vs_letter_prev:
    lda.w $0DB4,x               ; $8AAEB9 static
    pha                         ; $8AAEBC static
CODE_8AAEBD:
    lda.w $0DB4,x               ; $8AAEBD static
    beq CODE_8AAED5             ; $8AAEC0 static
    dec.w $0DB4,x               ; $8AAEC2 static
    jsr.w vs_letter_index       ; $8AAEC5 static
    lda.w $AFF1,y               ; $8AAEC8 static
    and.w #$00FF                ; $8AAECB static
    cmp.w #$0020                ; $8AAECE static
    beq CODE_8AAEBD             ; $8AAED1 static
    pla                         ; $8AAED3 static
    rts                         ; $8AAED4 static
CODE_8AAED5:
    pla                         ; $8AAED5 static
    sta.w $0DB4,x               ; $8AAED6 static
    rts                         ; $8AAED9 static
vs_letter_next:
    lda.w $0DB4,x               ; $8AAEDA
    pha                         ; $8AAEDD
CODE_8AAEDE:
    lda.w $0DB4,x               ; $8AAEDE
    cmp.w #$0004                ; $8AAEE1
    beq CODE_8AAEFE             ; $8AAEE4
    inc.w $0DB4,x               ; $8AAEE6 static
    jsr.w vs_letter_index       ; $8AAEE9 static
    lda.w $AFF1,y               ; $8AAEEC static
    and.w #$00FF                ; $8AAEEF static
    cmp.w #$0020                ; $8AAEF2 static
    beq CODE_8AAEDE             ; $8AAEF5 static
    cmp.w #$003C                ; $8AAEF7 static
    beq CODE_8AAF03             ; $8AAEFA static
    pla                         ; $8AAEFC static
    rts                         ; $8AAEFD static
CODE_8AAEFE:
    pla                         ; $8AAEFE
    sta.w $0DB4,x               ; $8AAEFF
    rts                         ; $8AAF02
CODE_8AAF03:
    dec.w $0DAC,x               ; $8AAF03 static
    bra CODE_8AAEDE             ; $8AAF06 static
vs_row_next:
    lda.w $0DAC,x               ; $8AAF08
    pha                         ; $8AAF0B
CODE_8AAF0C:
    lda.w $0DAC,x               ; $8AAF0C
    cmp.w #$0006                ; $8AAF0F
    beq CODE_8AAF2C             ; $8AAF12
    inc.w $0DAC,x               ; $8AAF14
    jsr.w vs_letter_index       ; $8AAF17
    lda.w $AFF1,y               ; $8AAF1A
    and.w #$00FF                ; $8AAF1D
    cmp.w #$0020                ; $8AAF20
    beq CODE_8AAF0C             ; $8AAF23
    cmp.w #$003C                ; $8AAF25
    beq CODE_8AAF0C             ; $8AAF28
    pla                         ; $8AAF2A static
    rts                         ; $8AAF2B static
CODE_8AAF2C:
    pla                         ; $8AAF2C
    sta.w $0DAC,x               ; $8AAF2D
    rts                         ; $8AAF30
vs_row_prev:
    lda.w $0DAC,x               ; $8AAF31 static
    pha                         ; $8AAF34 static
CODE_8AAF35:
    lda.w $0DAC,x               ; $8AAF35 static
    beq CODE_8AAF52             ; $8AAF38 static
    dec.w $0DAC,x               ; $8AAF3A static
    jsr.w vs_letter_index       ; $8AAF3D static
    lda.w $AFF1,y               ; $8AAF40 static
    and.w #$00FF                ; $8AAF43 static
    cmp.w #$0020                ; $8AAF46 static
    beq CODE_8AAF35             ; $8AAF49 static
    cmp.w #$003C                ; $8AAF4B static
    beq CODE_8AAF35             ; $8AAF4E static
    pla                         ; $8AAF50 static
    rts                         ; $8AAF51 static
CODE_8AAF52:
    pla                         ; $8AAF52 static
    sta.w $0DAC,x               ; $8AAF53 static
    rts                         ; $8AAF56 static
    incbin !baserom:$52F57..$53037
vs_grid_draw:
    stx.b $3A                   ; $8AB037
    sty.b $3C                   ; $8AB039
    lda.w #$AFAB                ; $8AB03B
    jsr.w vs_put_string         ; $8AB03E
    stx.b $3A                   ; $8AB041
    inc.b $3C                   ; $8AB043
    inc.b $3C                   ; $8AB045
    lda.w #$AFB9                ; $8AB047
    jsr.w vs_put_string         ; $8AB04A
    stx.b $3A                   ; $8AB04D
    inc.b $3C                   ; $8AB04F
    inc.b $3C                   ; $8AB051
    lda.w #$AFC7                ; $8AB053
    jsr.w vs_put_string         ; $8AB056
    stx.b $3A                   ; $8AB059
    inc.b $3C                   ; $8AB05B
    inc.b $3C                   ; $8AB05D
    lda.w #$AFD5                ; $8AB05F
    jsr.w vs_put_string         ; $8AB062
    stx.b $3A                   ; $8AB065
    inc.b $3C                   ; $8AB067
    inc.b $3C                   ; $8AB069
    lda.w #$AFE3                ; $8AB06B
    jsr.w vs_put_string         ; $8AB06E
    rts                         ; $8AB071
vs_put_string:
    phx                         ; $8AB072
    phy                         ; $8AB073
    stz.b $8A                   ; $8AB074
    sta.b $44                   ; $8AB076
    sep.b #$20                  ; $8AB078
    ldy.w #$0000                ; $8AB07A
CODE_8AB07D:
    lda.b ($44),y               ; $8AB07D
    beq CODE_8AB087             ; $8AB07F
    jsr.w vs_put_char           ; $8AB081
    iny                         ; $8AB084
    bra CODE_8AB07D             ; $8AB085
CODE_8AB087:
    rep.b #$20                  ; $8AB087
    ply                         ; $8AB089
    plx                         ; $8AB08A
    rts                         ; $8AB08B
menu_action_difficulty:
    lda.w $1526                 ; $8AB08C
    lsr a                       ; $8AB08F
    lsr a                       ; $8AB090
    and.w #$000F                ; $8AB091
    cmp.w #$0002                ; $8AB094
    bne CODE_8AB09C             ; $8AB097
    lda.w #$FFFF                ; $8AB099
CODE_8AB09C:
    inc a                       ; $8AB09C
    asl a                       ; $8AB09D
    asl a                       ; $8AB09E
    sta.b $6A                   ; $8AB09F
    lda.w $1526                 ; $8AB0A1
    and.w #$FFC3                ; $8AB0A4
    ora.b $6A                   ; $8AB0A7
    sta.w $1526                 ; $8AB0A9
    rts                         ; $8AB0AC
menu_action_music:
    lda.w $1528                 ; $8AB0AD
    lsr a                       ; $8AB0B0
    lsr a                       ; $8AB0B1
    and.w #$000F                ; $8AB0B2
    cmp.w #$0001                ; $8AB0B5
    bne CODE_8AB0BF             ; $8AB0B8
    lda.w #$0000                ; $8AB0BA
    bra CODE_8AB0C2             ; $8AB0BD
CODE_8AB0BF:
    inc a                       ; $8AB0BF
    asl a                       ; $8AB0C0
    asl a                       ; $8AB0C1
CODE_8AB0C2:
    sta.b $6A                   ; $8AB0C2
    lda.w $1528                 ; $8AB0C4
    and.w #$FFC3                ; $8AB0C7
    ora.b $6A                   ; $8AB0CA
    sta.w $1528                 ; $8AB0CC
    rts                         ; $8AB0CF
menu_action_exit:
    inc.b $5E                   ; $8AB0D0
    rts                         ; $8AB0D2
menu_action_options:
    lda.w #$CC83                ; $8AB0D3
    jsr.w menu_submenu_open     ; $8AB0D6
    jsr.w menu_options_apply    ; $8AB0D9
    jsl.l save_data_seal        ; $8AB0DC
    rts                         ; $8AB0E0
variant_flags_apply:
    lda.w $1528                 ; $8AB0E1
    and.w #$FFC3                ; $8AB0E4
    sta.w $1528                 ; $8AB0E7
    lda.l $8103DC               ; $8AB0EA
    and.w #$0400                ; $8AB0EE
    ldy.w #$0008                ; $8AB0F1
    jsr.w lsr_n                 ; $8AB0F4
    ora.w $1528                 ; $8AB0F7
    sta.w $1528                 ; $8AB0FA
    lda.w $1526                 ; $8AB0FD
    and.w #$FFC3                ; $8AB100
    sta.w $1526                 ; $8AB103
    lda.l $8103DC               ; $8AB106
    and.w #$0300                ; $8AB10A
    ldy.w #$0006                ; $8AB10D
    jsr.w lsr_n                 ; $8AB110
    ora.w $1526                 ; $8AB113
    sta.w $1526                 ; $8AB116
    rts                         ; $8AB119
menu_options_apply:
    lda.l $8103DC               ; $8AB11A
    and.w #$F8FF                ; $8AB11E
    sta.l $8103DC               ; $8AB121
    lda.w $1528                 ; $8AB125
    lsr a                       ; $8AB128
    lsr a                       ; $8AB129
    and.w #$0001                ; $8AB12A
    ldy.w #$000A                ; $8AB12D
    jsr.w asl_n                 ; $8AB130
    ora.l $8103DC               ; $8AB133
    sta.l $8103DC               ; $8AB137
    lda.w $1526                 ; $8AB13B
    lsr a                       ; $8AB13E
    lsr a                       ; $8AB13F
    and.w #$0003                ; $8AB140
    ldy.w #$0008                ; $8AB143
    jsr.w asl_n                 ; $8AB146
    ora.l $8103DC               ; $8AB149
    sta.l $8103DC               ; $8AB14D
    rts                         ; $8AB151
asl_n:
    phy                         ; $8AB152
    cpy.w #$0000                ; $8AB153
    beq CODE_8AB15C             ; $8AB156
CODE_8AB158:
    asl a                       ; $8AB158
    dey                         ; $8AB159
    bne CODE_8AB158             ; $8AB15A
CODE_8AB15C:
    ply                         ; $8AB15C
    rts                         ; $8AB15D
lsr_n:
    cpy.w #$0000                ; $8AB15E
    beq CODE_8AB167             ; $8AB161
CODE_8AB163:
    lsr a                       ; $8AB163
    dey                         ; $8AB164
    bne CODE_8AB163             ; $8AB165
CODE_8AB167:
    rts                         ; $8AB167
    incbin !baserom:$53168..$53174
menu_action_rookie:
    sep.b #$20                  ; $8AB174
    lda.b #$10                  ; $8AB176
    sta.w $03DC                 ; $8AB178
    rep.b #$20                  ; $8AB17B
    inc.b $6C                   ; $8AB17D
    rts                         ; $8AB17F
    incbin !baserom:$53180..$5318C
password_hdma_off:
    lda.w #$0001                ; $8AB18C
    jsl.l wait_frames_long      ; $8AB18F
    stz.w $420C                 ; $8AB193
    rts                         ; $8AB196
password_hdma_on:
    lda.w #$0001                ; $8AB197
    jsl.l wait_frames_long      ; $8AB19A
    lda.w #$0082                ; $8AB19E
    sta.w $420C                 ; $8AB1A1
    rts                         ; $8AB1A4
password_screen_open:
    jsr.w fade_out_wait         ; $8AB1A5
    jsr.w password_hdma_off     ; $8AB1A8
    jsr.w wipe_columns          ; $8AB1AB
    stz.b $8E                   ; $8AB1AE
    jsr.w wait_one_frame        ; $8AB1B0
    ldx.w #$8601                ; $8AB1B3
    ldy.w #$1000                ; $8AB1B6
    lda.w #$0088                ; $8AB1B9
    jsl.l decompress_to_vram_long; $8AB1BC
    jsr.w func_8ABA55           ; $8AB1C0
    lda.w #$8560                ; $8AB1C3
    ldx.w #$FA37                ; $8AB1C6
    ldy.w #$0040                ; $8AB1C9
    jsl.l upload_cgram_now      ; $8AB1CC
    jsr.w wipe_columns          ; $8AB1D0
    jsr.w password_hdma_on      ; $8AB1D3
    jsr.w fade_in_start         ; $8AB1D6
    jsr.w wait_one_frame        ; $8AB1D9
    inc.b $8E                   ; $8AB1DC
    inc.w $0D7C                 ; $8AB1DE
    rts                         ; $8AB1E1
password_screen_close:
    jsr.w fade_out_wait         ; $8AB1E2
    jsr.w password_hdma_off     ; $8AB1E5
    jsr.w wipe_columns          ; $8AB1E8
    stz.b $8E                   ; $8AB1EB
    ldx.w #$EE52                ; $8AB1ED
    ldy.w #$4000                ; $8AB1F0
    lda.w #$0096                ; $8AB1F3
    jsl.l decompress_to_vram_long; $8AB1F6
    jsr.w func_8ABA55           ; $8AB1FA
    jsr.w func_8AB994           ; $8AB1FD
    jsr.w wipe_columns          ; $8AB200
    jsr.w password_hdma_on      ; $8AB203
    jsr.w fade_in_start         ; $8AB206
    jsr.w wait_one_frame        ; $8AB209
    inc.b $8E                   ; $8AB20C
    stz.w $0D7C                 ; $8AB20E
    rts                         ; $8AB211
wait_one_frame:
    pha                         ; $8AB212
    lda.w #$0001                ; $8AB213
    jsl.l wait_frames_long      ; $8AB216
    pla                         ; $8AB21A
    rts                         ; $8AB21B
    jsr.w wipe_columns          ; $8AB21C
    jsr.w password_screen_open  ; $8AB21F
    jsr.w password_entry_open   ; $8AB222
menu_password_back:
    lda.w $0D7C                 ; $8AB225
    db $F0,$04 ; beq            ; $8AB228
    jsr.w password_screen_close ; $8AB22A
    rts                         ; $8AB22D
    incbin !baserom:$5322E..$53232
password_entry_open:
    stz.b $52                   ; $8AB232
    jsr.w hdma_window_tables_init; $8AB234
    jsr.w password_lines_print  ; $8AB237
    ldx.w #$0017                ; $8AB23A
CODE_8AB23D:
    stz.w $0D40,x               ; $8AB23D
    dex                         ; $8AB240
    dex                         ; $8AB241
    bpl CODE_8AB23D             ; $8AB242
    lda.w #$FFFF                ; $8AB244
    sta.w $0578                 ; $8AB247
    lda.w #$3002                ; $8AB24A
    sta.w $0878                 ; $8AB24D
    lda.w #$0019                ; $8AB250
    sta.b $3C                   ; $8AB253
    stz.b $80                   ; $8AB255
    lda.w #$0006                ; $8AB257
    sta.w $0D78                 ; $8AB25A
    lda.w #$FF8C                ; $8AB25D
    sta.w $0D88                 ; $8AB260
    lda.w #$0064                ; $8AB263
    sta.w $0D90                 ; $8AB266
    jsr.w password_text_draw    ; $8AB269
CODE_8AB26C:
    lda.w #$0001                ; $8AB26C
    jsl.l wait_frames_long      ; $8AB26F
password_entry_sprites:
    jsr.w clear_object_slots    ; $8AB273
    ldx.w #$0000                ; $8AB276
    jsr.w sprite_frame          ; $8AB279
    lda.w $0D88                 ; $8AB27C
    cmp.w $0D80                 ; $8AB27F
    beq password_entry_ready    ; $8AB282
password_entry_slide:
    jsr.w password_caret_place  ; $8AB284
    jsr.w password_caret_blink  ; $8AB287
    bra CODE_8AB26C             ; $8AB28A
password_entry_ready:
    lda.w #$0002                ; $8AB28C
    sta.b $7A                   ; $8AB28F
    sta.b $7C                   ; $8AB291
    sta.b $7E                   ; $8AB293
    lda.w #$0001                ; $8AB295
    jsl.l wait_frames_long      ; $8AB298
password_entry_frame:
    jsr.w clear_object_slots    ; $8AB29C
    ldx.w #$0000                ; $8AB29F
    jsr.w sprite_frame          ; $8AB2A2
    jsr.w password_press_a      ; $8AB2A5
    bcs password_entry_check    ; $8AB2A8
password_entry_pads:
    jsr.w password_pad_right    ; $8AB2AA
    jsr.w password_pad_left     ; $8AB2AD
    jsr.w password_pad_up       ; $8AB2B0
    jsr.w password_pad_down     ; $8AB2B3
    jsr.w password_caret_blink  ; $8AB2B6
    jmp.w $B295                 ; $8AB2B9
password_caret_blink:
    lda.b $66                   ; $8AB2BC
    and.w #$0001                ; $8AB2BE
    bne CODE_8AB2CA             ; $8AB2C1
    lda.w #$FFFF                ; $8AB2C3
    sta.w $0578                 ; $8AB2C6
    rts                         ; $8AB2C9
CODE_8AB2CA:
    stz.w $0578                 ; $8AB2CA
    rts                         ; $8AB2CD
password_entry_check:
    lda.w #$0004                ; $8AB2CE
    sta.b $7A                   ; $8AB2D1
    sta.b $7C                   ; $8AB2D3
    lda.w #$0007                ; $8AB2D5
    sta.b $7E                   ; $8AB2D8
    stz.w $0D88                 ; $8AB2DA
    stz.w $0578                 ; $8AB2DD
    jsr.w password_decode       ; $8AB2E0
    jsr.w password_checksum_ok  ; $8AB2E3
    bcc CODE_8AB31B             ; $8AB2E6
    jsr.w password_code_find    ; $8AB2E8
    bcs password_entry_found    ; $8AB2EB
    incbin !baserom:$532ED..$532FD
password_entry_found:
    lda.b $52                   ; $8AB2FD
    db $D0,$01 ; bne            ; $8AB2FF
    rts                         ; $8AB301
    incbin !baserom:$53302..$5331B
CODE_8AB31B:
    lda.w #$79FF                ; $8AB31B
    trb.w $03DC                 ; $8AB31E
    jsr.w password_unpack       ; $8AB321
    lda.w #$0010                ; $8AB324
    tsb.w $03DC                 ; $8AB327
    jsr.w func_8ABF76           ; $8AB32A
    inc.b $6C                   ; $8AB32D
    lda.w $03DC                 ; $8AB32F
    ora.w #$4000                ; $8AB332
    sta.w $03DC                 ; $8AB335
    rts                         ; $8AB338
password_press_a:
    lda.w #$C0C0                ; $8AB339
    trb.w $000E                 ; $8AB33C
    bne CODE_8AB3A7             ; $8AB33F
    lda.w #$C0C0                ; $8AB341
    trb.w $0012                 ; $8AB344
    bne CODE_8AB3A7             ; $8AB347
    lda.w #$C0C0                ; $8AB349
    trb.w $0016                 ; $8AB34C
    bne CODE_8AB3A7             ; $8AB34F
    lda.w #$C0C0                ; $8AB351
    trb.w $001A                 ; $8AB354
    bne CODE_8AB3A7             ; $8AB357
    clc                         ; $8AB359
    rts                         ; $8AB35A
password_code_find:
    sep.b #$20                  ; $8AB35B
    ldx.w #$0000                ; $8AB35D
CODE_8AB360:
    ldy.w $CDAB,x               ; $8AB360
    db $F0,$34 ; beq            ; $8AB363
    sty.b $82                   ; $8AB365
    inx                         ; $8AB367
    inx                         ; $8AB368
    jsr.w $B37B                 ; $8AB369
    bcs CODE_8AB39D             ; $8AB36C
CODE_8AB36E:
    lda.w $CDAB,x               ; $8AB36E
    cmp.b #$00                  ; $8AB371
    beq CODE_8AB378             ; $8AB373
    inx                         ; $8AB375
    bra CODE_8AB36E             ; $8AB376
CODE_8AB378:
    inx                         ; $8AB378
    bra CODE_8AB360             ; $8AB379
    phx                         ; $8AB37B
    ldy.w #$0000                ; $8AB37C
CODE_8AB37F:
    lda.w $CDAB,x               ; $8AB37F
    beq CODE_8AB386             ; $8AB382
    eor.b #$9A                  ; $8AB384
CODE_8AB386:
    cmp.w $0D40,y               ; $8AB386
    bne CODE_8AB396             ; $8AB389
    cmp.b #$00                  ; $8AB38B
    beq CODE_8AB393             ; $8AB38D
    inx                         ; $8AB38F
    iny                         ; $8AB390
    bra CODE_8AB37F             ; $8AB391
CODE_8AB393:
    plx                         ; $8AB393
    sec                         ; $8AB394
    rts                         ; $8AB395
CODE_8AB396:
    plx                         ; $8AB396
    clc                         ; $8AB397
    rts                         ; $8AB398
    incbin !baserom:$53399..$5339D
CODE_8AB39D:
    rep.b #$20                  ; $8AB39D
    jsr.w $B3A4                 ; $8AB39F
    sec                         ; $8AB3A2
    rts                         ; $8AB3A3
    jmp.w ($0182)               ; $8AB3A4
CODE_8AB3A7:
    jsr.w password_press_last_column; $8AB3A7
    bcs CODE_8AB3DF             ; $8AB3AA
    lda.w $0D80                 ; $8AB3AC
    clc                         ; $8AB3AF
    adc.w #$0074                ; $8AB3B0
    pha                         ; $8AB3B3
    and.w #$000F                ; $8AB3B4
    bne CODE_8AB3E0             ; $8AB3B7
    pla                         ; $8AB3B9
    lsr a                       ; $8AB3BA
    lsr a                       ; $8AB3BB
    lsr a                       ; $8AB3BC
    lsr a                       ; $8AB3BD
    tay                         ; $8AB3BE
    lda.w $0D80                 ; $8AB3BF
    cmp.w #$003D                ; $8AB3C2
    bpl CODE_8AB3DE             ; $8AB3C5
    lda.w $0D90                 ; $8AB3C7
    cmp.w #$0064                ; $8AB3CA
    beq CODE_8AB3E3             ; $8AB3CD
    cmp.w #$007C                ; $8AB3CF
    beq CODE_8AB3EB             ; $8AB3D2
    cmp.w #$0094                ; $8AB3D4
    beq CODE_8AB3F3             ; $8AB3D7
    cmp.w #$00AC                ; $8AB3D9
    beq CODE_8AB3FB             ; $8AB3DC
CODE_8AB3DE:
    clc                         ; $8AB3DE static
CODE_8AB3DF:
    rts                         ; $8AB3DF
CODE_8AB3E0:
    pla                         ; $8AB3E0 static
    clc                         ; $8AB3E1 static
    rts                         ; $8AB3E2 static
CODE_8AB3E3:
    lda.w $C572,y               ; $8AB3E3
    jsr.w password_type_char    ; $8AB3E6
    clc                         ; $8AB3E9
    rts                         ; $8AB3EA
CODE_8AB3EB:
    lda.w $C57E,y               ; $8AB3EB
    jsr.w password_type_char    ; $8AB3EE
    clc                         ; $8AB3F1
    rts                         ; $8AB3F2
CODE_8AB3F3:
    lda.w $C58A,y               ; $8AB3F3
    jsr.w password_type_char    ; $8AB3F6
    clc                         ; $8AB3F9
    rts                         ; $8AB3FA
CODE_8AB3FB:
    lda.w $C596,y               ; $8AB3FB
    jsr.w password_type_char    ; $8AB3FE
    clc                         ; $8AB401
    rts                         ; $8AB402
password_type_char:
    sep.b #$20                  ; $8AB403
    ldy.b $80                   ; $8AB405
    cpy.w #$0015                ; $8AB407
    beq CODE_8AB41A             ; $8AB40A
    sta.w $0D40,y               ; $8AB40C
    inc.b $80                   ; $8AB40F
    rep.b #$20                  ; $8AB411
    jsr.w password_text_draw    ; $8AB413
    jsr.w func_8AA48D           ; $8AB416
    rts                         ; $8AB419
CODE_8AB41A:
    jsr.w func_8AA4AF           ; $8AB41A static
    rep.b #$20                  ; $8AB41D static
    rts                         ; $8AB41F static
password_delete:
    lda.b $80                   ; $8AB420 static
    beq CODE_8AB439             ; $8AB422 static
    dec.b $80                   ; $8AB424 static
    lda.b $80                   ; $8AB426 static
    tay                         ; $8AB428 static
    sep.b #$20                  ; $8AB429 static
    lda.b #$00                  ; $8AB42B static
    sta.w $0D40,y               ; $8AB42D static
    rep.b #$20                  ; $8AB430 static
    jsr.w password_text_draw    ; $8AB432 static
    jsr.w func_8AA48D           ; $8AB435 static
    rts                         ; $8AB438 static
CODE_8AB439:
    jsr.w func_8AA4AF           ; $8AB439 static
    rts                         ; $8AB43C static
password_text_draw:
    lda.w #$0002                ; $8AB43D
    sta.b $3A                   ; $8AB440
    lda.b $80                   ; $8AB442
    beq CODE_8AB470             ; $8AB444
    stz.b $48                   ; $8AB446
    ldx.w #$0000                ; $8AB448
CODE_8AB44B:
    lda.l $810D40,x             ; $8AB44B
    and.w #$00FF                ; $8AB44F
    phx                         ; $8AB452
    jsr.w text_put_char         ; $8AB453
    inc.b $48                   ; $8AB456
    lda.b $48                   ; $8AB458
    and.w #$0003                ; $8AB45A
    bne CODE_8AB46A             ; $8AB45D
    jsr.w password_no_vowel     ; $8AB45F
    bcc CODE_8AB46A             ; $8AB462
    lda.w #$0020                ; $8AB464
    jsr.w text_put_char         ; $8AB467
CODE_8AB46A:
    plx                         ; $8AB46A
    inx                         ; $8AB46B
    cpx.b $80                   ; $8AB46C
    bne CODE_8AB44B             ; $8AB46E
CODE_8AB470:
    lda.w #$0020                ; $8AB470
    jsr.w text_put_char         ; $8AB473
    dec.b $3A                   ; $8AB476
    jsr.w password_caret_place  ; $8AB478
    rts                         ; $8AB47B
password_no_vowel:
    sep.b #$20                  ; $8AB47C
    ldx.w #$0003                ; $8AB47E
CODE_8AB481:
    lda.l $810D40,x             ; $8AB481
    cmp.b #$41                  ; $8AB485
    beq CODE_8AB4A0             ; $8AB487
    cmp.b #$45                  ; $8AB489
    beq CODE_8AB4A0             ; $8AB48B
    cmp.b #$49                  ; $8AB48D
    beq CODE_8AB4A0             ; $8AB48F
    cmp.b #$4F                  ; $8AB491
    beq CODE_8AB4A0             ; $8AB493
    cmp.b #$55                  ; $8AB495
    beq CODE_8AB4A0             ; $8AB497
    dex                         ; $8AB499
    bpl CODE_8AB481             ; $8AB49A
    rep.b #$20                  ; $8AB49C
    sec                         ; $8AB49E
    rts                         ; $8AB49F
CODE_8AB4A0:
    rep.b #$20                  ; $8AB4A0
    clc                         ; $8AB4A2
    rts                         ; $8AB4A3
password_press_last_column:
    lda.w $0D80                 ; $8AB4A4
    cmp.w #$0064                ; $8AB4A7
    bne CODE_8AB4BE             ; $8AB4AA
    lda.w $0D68                 ; $8AB4AC
    cmp.w #$0094                ; $8AB4AF
    beq CODE_8AB4CD             ; $8AB4B2
    cmp.w #$007C                ; $8AB4B4 static
    beq CODE_8AB4C8             ; $8AB4B7 static
    cmp.w #$0064                ; $8AB4B9 static
    beq CODE_8AB4C0             ; $8AB4BC static
CODE_8AB4BE:
    clc                         ; $8AB4BE
    rts                         ; $8AB4BF
CODE_8AB4C0:
    lda.w #$0020                ; $8AB4C0 static
    jsr.w password_type_char    ; $8AB4C3 static
    clc                         ; $8AB4C6 static
    rts                         ; $8AB4C7 static
CODE_8AB4C8:
    jsr.w password_delete       ; $8AB4C8 static
    clc                         ; $8AB4CB static
    rts                         ; $8AB4CC static
CODE_8AB4CD:
    jsr.w func_8AA49E           ; $8AB4CD
    sec                         ; $8AB4D0
    rts                         ; $8AB4D1
password_pad_right:
    lda.w #$0100                ; $8AB4D2
    and.w $000E                 ; $8AB4D5
    bne CODE_8AB4F3             ; $8AB4D8
    lda.w #$0100                ; $8AB4DA
    and.w $0012                 ; $8AB4DD
    bne CODE_8AB4F3             ; $8AB4E0
    lda.w #$0100                ; $8AB4E2
    and.w $0016                 ; $8AB4E5
    bne CODE_8AB4F3             ; $8AB4E8
    lda.w #$0100                ; $8AB4EA
    and.w $001A                 ; $8AB4ED
    bne CODE_8AB4F3             ; $8AB4F0
    rts                         ; $8AB4F2
CODE_8AB4F3:
    lda.w $0D88                 ; $8AB4F3
    cmp.w $0D80                 ; $8AB4F6
    bne CODE_8AB4FE             ; $8AB4F9
    jsr.w password_cursor_right ; $8AB4FB
CODE_8AB4FE:
    rts                         ; $8AB4FE
password_pad_left:
    lda.w #$0200                ; $8AB4FF
    and.w $000E                 ; $8AB502
    bne CODE_8AB520             ; $8AB505
    lda.w #$0200                ; $8AB507
    and.w $0012                 ; $8AB50A
    bne CODE_8AB520             ; $8AB50D
    lda.w #$0200                ; $8AB50F
    and.w $0016                 ; $8AB512
    bne CODE_8AB520             ; $8AB515
    lda.w #$0200                ; $8AB517
    and.w $001A                 ; $8AB51A
    bne CODE_8AB520             ; $8AB51D
    rts                         ; $8AB51F
CODE_8AB520:
    lda.w $0D88                 ; $8AB520
    cmp.w $0D80                 ; $8AB523
    bne CODE_8AB52B             ; $8AB526
    jsr.w password_cursor_left  ; $8AB528
CODE_8AB52B:
    rts                         ; $8AB52B
password_pad_up:
    lda.w #$0800                ; $8AB52C
    and.w $000E                 ; $8AB52F
    bne CODE_8AB54D             ; $8AB532
    lda.w #$0800                ; $8AB534
    and.w $0012                 ; $8AB537
    bne CODE_8AB54D             ; $8AB53A
    lda.w #$0800                ; $8AB53C
    and.w $0016                 ; $8AB53F
    bne CODE_8AB54D             ; $8AB542
    lda.w #$0800                ; $8AB544
    and.w $001A                 ; $8AB547
    bne CODE_8AB54D             ; $8AB54A
    rts                         ; $8AB54C
CODE_8AB54D:
    lda.w $0D90                 ; $8AB54D
    cmp.w $0D68                 ; $8AB550
    bne CODE_8AB558             ; $8AB553
    jsr.w password_cursor_up    ; $8AB555
CODE_8AB558:
    rts                         ; $8AB558
password_pad_down:
    lda.w #$0400                ; $8AB559
    and.w $000E                 ; $8AB55C
    bne CODE_8AB57A             ; $8AB55F
    lda.w #$0400                ; $8AB561
    and.w $0012                 ; $8AB564
    bne CODE_8AB57A             ; $8AB567
    lda.w #$0400                ; $8AB569
    and.w $0016                 ; $8AB56C
    bne CODE_8AB57A             ; $8AB56F
    lda.w #$0400                ; $8AB571
    and.w $001A                 ; $8AB574
    bne CODE_8AB57A             ; $8AB577
    rts                         ; $8AB579
CODE_8AB57A:
    lda.w $0D90                 ; $8AB57A
    cmp.w $0D68                 ; $8AB57D
    bne CODE_8AB585             ; $8AB580
    jsr.w password_cursor_down  ; $8AB582
CODE_8AB585:
    rts                         ; $8AB585
password_cursor_right:
    lda.w $0D90                 ; $8AB586
    cmp.w #$00AC                ; $8AB589
    bne CODE_8AB596             ; $8AB58C
    lda.w $0D88                 ; $8AB58E static
    cmp.w #$FFA4                ; $8AB591 static
    bpl CODE_8AB5BE             ; $8AB594 static
CODE_8AB596:
    lda.w $0D88                 ; $8AB596
    cmp.w #$003C                ; $8AB599
    bpl CODE_8AB5AC             ; $8AB59C
    clc                         ; $8AB59E
    adc.w #$0010                ; $8AB59F
    sta.w $0D88                 ; $8AB5A2
    lda.w #$0006                ; $8AB5A5
    sta.w $0D78                 ; $8AB5A8
    rts                         ; $8AB5AB
CODE_8AB5AC:
    cmp.w #$0064                ; $8AB5AC
    beq CODE_8AB5BE             ; $8AB5AF
    clc                         ; $8AB5B1
    adc.w #$0028                ; $8AB5B2
    sta.w $0D88                 ; $8AB5B5
    lda.w #$0026                ; $8AB5B8
    sta.w $0D78                 ; $8AB5BB
CODE_8AB5BE:
    rts                         ; $8AB5BE
password_cursor_left:
    lda.w $0D88                 ; $8AB5BF
    cmp.w #$0064                ; $8AB5C2
    beq CODE_8AB5D4             ; $8AB5C5
    cmp.w #$FF8C                ; $8AB5C7
    beq CODE_8AB5D3             ; $8AB5CA
    sec                         ; $8AB5CC
    sbc.w #$0010                ; $8AB5CD
    sta.w $0D88                 ; $8AB5D0
CODE_8AB5D3:
    rts                         ; $8AB5D3
CODE_8AB5D4:
    sec                         ; $8AB5D4 static
    sbc.w #$0028                ; $8AB5D5 static
    sta.w $0D88                 ; $8AB5D8 static
    lda.w #$0006                ; $8AB5DB static
    sta.w $0D78                 ; $8AB5DE static
    rts                         ; $8AB5E1 static
password_cursor_up:
    lda.w $0D90                 ; $8AB5E2
    cmp.w #$0064                ; $8AB5E5
    beq CODE_8AB5F1             ; $8AB5E8
    sec                         ; $8AB5EA
    sbc.w #$0018                ; $8AB5EB
    sta.w $0D90                 ; $8AB5EE
CODE_8AB5F1:
    rts                         ; $8AB5F1
password_cursor_down:
    lda.w $0D90                 ; $8AB5F2
    cmp.w #$0094                ; $8AB5F5
    bne CODE_8AB602             ; $8AB5F8
    lda.w $0D88                 ; $8AB5FA
    cmp.w #$FFB4                ; $8AB5FD
    bpl CODE_8AB611             ; $8AB600
CODE_8AB602:
    lda.w $0D90                 ; $8AB602
    cmp.w #$00AC                ; $8AB605
    beq CODE_8AB611             ; $8AB608
    clc                         ; $8AB60A
    adc.w #$0018                ; $8AB60B
    sta.w $0D90                 ; $8AB60E
CODE_8AB611:
    rts                         ; $8AB611
password_lines_print:
    stz.b $68                   ; $8AB612
    lda.w #$000D                ; $8AB614
    sta.b $3C                   ; $8AB617
    lda.w #$C4F6                ; $8AB619
    jsr.w text_print_centered   ; $8AB61C
    inc.b $3C                   ; $8AB61F
    inc.b $3C                   ; $8AB621
    inc.b $3C                   ; $8AB623
    lda.w #$C515                ; $8AB625
    jsr.w text_print_centered   ; $8AB628
    inc.b $3C                   ; $8AB62B
    inc.b $3C                   ; $8AB62D
    inc.b $3C                   ; $8AB62F
    lda.w #$C534                ; $8AB631
    jsr.w text_print_centered   ; $8AB634
    inc.b $3C                   ; $8AB637
    inc.b $3C                   ; $8AB639
    inc.b $3C                   ; $8AB63B
    lda.w #$C553                ; $8AB63D
    jsr.w text_print_centered   ; $8AB640
    rts                         ; $8AB643
password_caret_place:
    lda.b $3A                   ; $8AB644
    asl a                       ; $8AB646
    asl a                       ; $8AB647
    asl a                       ; $8AB648
    sta.w $0678                 ; $8AB649
    lda.b $3C                   ; $8AB64C
    asl a                       ; $8AB64E
    asl a                       ; $8AB64F
    asl a                       ; $8AB650
    clc                         ; $8AB651
    adc.w #$000F                ; $8AB652
    sta.w $0778                 ; $8AB655
    rts                         ; $8AB658
menu_submenu_open:
    sta.b $62                   ; $8AB659
    jsr.w wipe_columns          ; $8AB65B
menu_submenu_tick:
    jsr.w timer_0d7e_tick       ; $8AB65E
    lda.b $6C                   ; $8AB661
    bne CODE_8AB67F             ; $8AB663
    lda.w #$0001                ; $8AB665
    jsl.l wait_frames_long      ; $8AB668
menu_submenu_frame:
    jsr.w func_8A9FA1           ; $8AB66C
    lda.b $62                   ; $8AB66F
    jsr.w menu_draw             ; $8AB671
menu_submenu_chosen:
    lda.b $5E                   ; $8AB674
    beq menu_submenu_tick       ; $8AB676
    stz.b $5E                   ; $8AB678
    stz.b $6E                   ; $8AB67A
    jsr.w wipe_columns          ; $8AB67C
CODE_8AB67F:
    rts                         ; $8AB67F
func_8AB680:
    phx                         ; $8AB680
    phy                         ; $8AB681
    inc a                       ; $8AB682
    inc a                       ; $8AB683
    inc a                       ; $8AB684
    inc a                       ; $8AB685
    sta.b $60                   ; $8AB686
CODE_8AB688:
    ldy.w #$0000                ; $8AB688
    lda.b ($60),y               ; $8AB68B
    beq CODE_8AB6A7             ; $8AB68D
    ldy.w #$0004                ; $8AB68F
    lda.b ($60),y               ; $8AB692
    tax                         ; $8AB694
    ldy.w #$0006                ; $8AB695
    lda.b ($60),y               ; $8AB698
    sta.w $1516,x               ; $8AB69A
    clc                         ; $8AB69D
    lda.b $60                   ; $8AB69E
    adc.w #$0008                ; $8AB6A0
    sta.b $60                   ; $8AB6A3
    bra CODE_8AB688             ; $8AB6A5
CODE_8AB6A7:
    ply                         ; $8AB6A7
    plx                         ; $8AB6A8
    rts                         ; $8AB6A9
text_print_centered:
    phx                         ; $8AB6AA
    pha                         ; $8AB6AB
    sec                         ; $8AB6AC
    lda.b $40                   ; $8AB6AD
    sbc.b $3E                   ; $8AB6AF
    lsr a                       ; $8AB6B1
    sta.b $42                   ; $8AB6B2
    pla                         ; $8AB6B4
    sta.b $44                   ; $8AB6B5
    sep.b #$20                  ; $8AB6B7
    ldy.w #$0000                ; $8AB6B9
CODE_8AB6BC:
    lda.b ($44),y               ; $8AB6BC
    beq CODE_8AB6C3             ; $8AB6BE
    iny                         ; $8AB6C0
    bra CODE_8AB6BC             ; $8AB6C1
CODE_8AB6C3:
    rep.b #$20                  ; $8AB6C3
    sty.b $5A                   ; $8AB6C5
    tya                         ; $8AB6C7
    lsr a                       ; $8AB6C8
    sta.b $46                   ; $8AB6C9
    sec                         ; $8AB6CB
    lda.b $42                   ; $8AB6CC
    sbc.b $46                   ; $8AB6CE
    clc                         ; $8AB6D0
    adc.b $3E                   ; $8AB6D1
    inc a                       ; $8AB6D3
    sta.b $3A                   ; $8AB6D4
    sep.b #$20                  ; $8AB6D6
    ldy.w #$0000                ; $8AB6D8
CODE_8AB6DB:
    lda.b $68                   ; $8AB6DB
    beq CODE_8AB6EB             ; $8AB6DD
    incbin !baserom:$536DF..$536EB
CODE_8AB6EB:
    lda.b ($44),y               ; $8AB6EB
    beq CODE_8AB6F5             ; $8AB6ED
    jsr.w text_put_char         ; $8AB6EF
    iny                         ; $8AB6F2
    bra CODE_8AB6DB             ; $8AB6F3
CODE_8AB6F5:
    rep.b #$20                  ; $8AB6F5
    lda.b $86                   ; $8AB6F7
    bne CODE_8AB6FE             ; $8AB6F9
    jsr.w func_8AB700           ; $8AB6FB
CODE_8AB6FE:
    plx                         ; $8AB6FE
    rts                         ; $8AB6FF
func_8AB700:
    phy                         ; $8AB700
    lda.b $3C                   ; $8AB701
    sec                         ; $8AB703
    sbc.w #$000A                ; $8AB704
    bmi CODE_8AB718             ; $8AB707
    tay                         ; $8AB709
    lda.b $5A                   ; $8AB70A
    and.w #$0001                ; $8AB70C
    beq CODE_8AB719             ; $8AB70F
    lda.w #$0004                ; $8AB711
    sta.w $0D04,y               ; $8AB714
    ply                         ; $8AB717
CODE_8AB718:
    rts                         ; $8AB718
CODE_8AB719:
    lda.w #$0000                ; $8AB719
    sta.w $0D04,y               ; $8AB71C
    ply                         ; $8AB71F
    rts                         ; $8AB720
vs_put_sprite_char:
    stx.b $3A                   ; $8AB721
    sty.b $3C                   ; $8AB723
    ldx.w #$0001                ; $8AB725
    stx.b $88                   ; $8AB728
    jsr.w vs_put_char           ; $8AB72A
    stz.b $88                   ; $8AB72D
    rts                         ; $8AB72F
vs_put_char:
    php                         ; $8AB730
    phy                         ; $8AB731
    phx                         ; $8AB732
    rep.b #$20                  ; $8AB733
    and.w #$00FF                ; $8AB735
    pha                         ; $8AB738
    lda.b $88                   ; $8AB739
    beq CODE_8AB78A             ; $8AB73B
    pla                         ; $8AB73D
    sec                         ; $8AB73E
    sbc.w #$0020                ; $8AB73F
    bne CODE_8AB755             ; $8AB742
    clc                         ; $8AB744
    lda.b $3A                   ; $8AB745
    adc.w #$0007                ; $8AB747
    sta.b $3A                   ; $8AB74A
    lda.b $8A                   ; $8AB74C
    adc.w #$0007                ; $8AB74E
    sta.b $8A                   ; $8AB751
    bra CODE_8AB786             ; $8AB753
CODE_8AB755:
    asl a                       ; $8AB755
    tay                         ; $8AB756
    phy                         ; $8AB757
    lda.w $CB31,y               ; $8AB758
    ora.w #$3E80                ; $8AB75B
    pha                         ; $8AB75E
    jsr.w take_slot             ; $8AB75F
    lda.b $3A                   ; $8AB762
    sta.w $0600,y               ; $8AB764
    lda.b $3C                   ; $8AB767
    sta.w $0700,y               ; $8AB769
    lda.w #$FFFF                ; $8AB76C
    sta.w $0500,y               ; $8AB76F
    pla                         ; $8AB772
    sta.w $0800,y               ; $8AB773
    ply                         ; $8AB776
    clc                         ; $8AB777
    lda.w $B7AC,y               ; $8AB778
    adc.b $3A                   ; $8AB77B
    sta.b $3A                   ; $8AB77D
    lda.b $8A                   ; $8AB77F
    adc.w $B7AC,y               ; $8AB781
    sta.b $8A                   ; $8AB784
CODE_8AB786:
    plx                         ; $8AB786
    ply                         ; $8AB787
    plp                         ; $8AB788
    rts                         ; $8AB789
CODE_8AB78A:
    pla                         ; $8AB78A
    ldy.b $3A                   ; $8AB78B
    sty.b $36                   ; $8AB78D
    ldy.b $3C                   ; $8AB78F
    sty.b $38                   ; $8AB791
    jsr.w tilemap_offset        ; $8AB793
    sec                         ; $8AB796
    sbc.w #$0020                ; $8AB797
    asl a                       ; $8AB79A
    tay                         ; $8AB79B
    lda.w $CB31,y               ; $8AB79C
    ora.w #$3880                ; $8AB79F
    sta.l $7F0000,x             ; $8AB7A2
    inc.b $3A                   ; $8AB7A6
    plx                         ; $8AB7A8
    ply                         ; $8AB7A9
    plp                         ; $8AB7AA
    rts                         ; $8AB7AB
    incbin !baserom:$537AC..$5383C
text_put_char:
    ldx.w $0D7C                 ; $8AB83C
    bne CODE_8AB844             ; $8AB83F
    jmp.w $B8C9                 ; $8AB841
CODE_8AB844:
    php                         ; $8AB844
    phy                         ; $8AB845
    rep.b #$20                  ; $8AB846
    and.w #$00FF                ; $8AB848
    ldy.b $3A                   ; $8AB84B
    sty.b $36                   ; $8AB84D
    ldy.b $3C                   ; $8AB84F
    sty.b $38                   ; $8AB851
    jsr.w tilemap_offset        ; $8AB853
    phx                         ; $8AB856
    phx                         ; $8AB857
    ldy.w #$3800                ; $8AB858
    sty.b $42                   ; $8AB85B
    and.w #$00FF                ; $8AB85D
    cmp.w #$0030                ; $8AB860
    bcc CODE_8AB883             ; $8AB863
    cmp.w #$003A                ; $8AB865
    bcs CODE_8AB874             ; $8AB868
    sbc.w #$0015                ; $8AB86A
    ldy.w #$3C00                ; $8AB86D
    sty.b $42                   ; $8AB870
    bra CODE_8AB8A1             ; $8AB872
CODE_8AB874:
    cmp.w #$0041                ; $8AB874
    bcc CODE_8AB883             ; $8AB877
    cmp.w #$005B                ; $8AB879
    bcs CODE_8AB883             ; $8AB87C
    sbc.w #$0040                ; $8AB87E
    bra CODE_8AB8A1             ; $8AB881
CODE_8AB883:
    cmp.w #$0020                ; $8AB883
    bne CODE_8AB88D             ; $8AB886
    lda.w #$0033                ; $8AB888
    bra CODE_8AB8A1             ; $8AB88B
CODE_8AB88D:
    ldx.w #$0024                ; $8AB88D
    sep.b #$20                  ; $8AB890
CODE_8AB892:
    cmp.l $8AB8D6,x             ; $8AB892
    beq CODE_8AB89E             ; $8AB896
    inx                         ; $8AB898
    cpx.w #$0033                ; $8AB899
    bne CODE_8AB892             ; $8AB89C
CODE_8AB89E:
    rep.b #$20                  ; $8AB89E
    txa                         ; $8AB8A0
CODE_8AB8A1:
    asl a                       ; $8AB8A1
    asl a                       ; $8AB8A2
    tax                         ; $8AB8A3
    lda.l $81CCD9,x             ; $8AB8A4
    txy                         ; $8AB8A8
    plx                         ; $8AB8A9
    ora.b $42                   ; $8AB8AA
    clc                         ; $8AB8AC
    adc.w #$0100                ; $8AB8AD
    sta.l $7F0000,x             ; $8AB8B0
    tyx                         ; $8AB8B4
    lda.l $81CCDB,x             ; $8AB8B5
    plx                         ; $8AB8B9
    ora.b $42                   ; $8AB8BA
    clc                         ; $8AB8BC
    adc.w #$0100                ; $8AB8BD
    sta.l $7F0040,x             ; $8AB8C0
    inc.b $3A                   ; $8AB8C4
    ply                         ; $8AB8C6
    plp                         ; $8AB8C7
    rts                         ; $8AB8C8
    php                         ; $8AB8C9
    phy                         ; $8AB8CA
    rep.b #$20                  ; $8AB8CB
    and.w #$00FF                ; $8AB8CD
    ldy.b $3A                   ; $8AB8D0
    sty.b $36                   ; $8AB8D2
    ldy.b $3C                   ; $8AB8D4
    sty.b $38                   ; $8AB8D6
    jsr.w tilemap_offset        ; $8AB8D8
    sec                         ; $8AB8DB
    sbc.w #$0020                ; $8AB8DC
    asl a                       ; $8AB8DF
    tay                         ; $8AB8E0
    lda.w $C9C4,y               ; $8AB8E1
    ora.w #$3C00                ; $8AB8E4
    sta.l $7F0000,x             ; $8AB8E7
    lda.w $CA3A,y               ; $8AB8EB
    ora.w #$3C00                ; $8AB8EE
    sta.l $7F0040,x             ; $8AB8F1
    inc.b $3A                   ; $8AB8F5
    ply                         ; $8AB8F7
    plp                         ; $8AB8F8
    rts                         ; $8AB8F9
    incbin !baserom:$538FA..$53909
tilemap_offset:
    pha                         ; $8AB909
    lda.b $38                   ; $8AB90A
    asl a                       ; $8AB90C
    asl a                       ; $8AB90D
    asl a                       ; $8AB90E
    asl a                       ; $8AB90F
    asl a                       ; $8AB910
    asl a                       ; $8AB911
    asl.b $36                   ; $8AB912
    clc                         ; $8AB914
    adc.b $36                   ; $8AB915
    tax                         ; $8AB917
    pla                         ; $8AB918
    rts                         ; $8AB919
wipe_columns:
    lda.w #$0000                ; $8AB91A
    sta.b $70                   ; $8AB91D
    lda.w #$007F                ; $8AB91F
    sta.b $72                   ; $8AB922
    stz.b $76                   ; $8AB924
    lda.w #$003E                ; $8AB926
    sta.b $74                   ; $8AB929
    ldx.w #$0010                ; $8AB92B
CODE_8AB92E:
    stz.b $78                   ; $8AB92E
    jsr.w func_8AB942           ; $8AB930
    lda.b $78                   ; $8AB933
    beq CODE_8AB93E             ; $8AB935
    lda.w #$0001                ; $8AB937
    jsl.l wait_frames_long      ; $8AB93A
CODE_8AB93E:
    dex                         ; $8AB93E
    bne CODE_8AB92E             ; $8AB93F
    rts                         ; $8AB941
func_8AB942:
    lda.b $70                   ; $8AB942
    pha                         ; $8AB944
    phx                         ; $8AB945
    ldx.w #$001C                ; $8AB946
CODE_8AB949:
    ldy.b $76                   ; $8AB949
    lda.b [$70],y               ; $8AB94B
    cmp.w #$003D                ; $8AB94D
    beq CODE_8AB954             ; $8AB950
    inc.b $78                   ; $8AB952
CODE_8AB954:
    lda.w #$003D                ; $8AB954
    sta.b [$70],y               ; $8AB957
    ldy.b $74                   ; $8AB959
    lda.b [$70],y               ; $8AB95B
    cmp.w #$003D                ; $8AB95D
    beq CODE_8AB964             ; $8AB960
    inc.b $78                   ; $8AB962
CODE_8AB964:
    lda.w #$003D                ; $8AB964
    sta.b [$70],y               ; $8AB967
    lda.b $70                   ; $8AB969
    clc                         ; $8AB96B
    adc.w #$0040                ; $8AB96C
    sta.b $70                   ; $8AB96F
    dex                         ; $8AB971
    bne CODE_8AB949             ; $8AB972
    plx                         ; $8AB974
    pla                         ; $8AB975
    sta.b $70                   ; $8AB976
    inc.b $76                   ; $8AB978
    inc.b $76                   ; $8AB97A
    dec.b $74                   ; $8AB97C
    dec.b $74                   ; $8AB97E
    rts                         ; $8AB980
clear_tilemap_7f:
    ldy.w #$0800                ; $8AB981
    ldx.w #$0000                ; $8AB984
    lda.w #$003D                ; $8AB987
CODE_8AB98A:
    sta.l $7F0000,x             ; $8AB98A
    inx                         ; $8AB98E
    inx                         ; $8AB98F
    dey                         ; $8AB990
    bne CODE_8AB98A             ; $8AB991
    rts                         ; $8AB993
func_8AB994:
    ldy.w #$0800                ; $8AB994
    ldx.w #$FEC3                ; $8AB997
    lda.w #$0088                ; $8AB99A
    jsl.l decompress_to_vram_long; $8AB99D
    ldy.w #$1000                ; $8AB9A1
    ldx.w #$F87F                ; $8AB9A4
    lda.w #$0088                ; $8AB9A7
    jsl.l decompress_to_vram_long; $8AB9AA
    rts                         ; $8AB9AE
vs_graphics_load:
    ldx.w #$F696                ; $8AB9AF
    lda.w #$008D                ; $8AB9B2
    ldy.w #$4000                ; $8AB9B5
    jsl.l decompress_to_vram_long; $8AB9B8
    ldx.w #$C86C                ; $8AB9BC
    ldy.w #$0000                ; $8AB9BF
    lda.w #$7F8A                ; $8AB9C2
    jsl.l decompress            ; $8AB9C5
    lda.w #$7000                ; $8AB9C9
    sta.w $2116                 ; $8AB9CC
    ldx.w #$0400                ; $8AB9CF
    lda.w #$0023                ; $8AB9D2
CODE_8AB9D5:
    sta.w $2118                 ; $8AB9D5
    dex                         ; $8AB9D8
    bne CODE_8AB9D5             ; $8AB9D9
    lda.w #$7000                ; $8AB9DB
    sta.w $2116                 ; $8AB9DE
    lda.w #$000E                ; $8AB9E1
    sta.b $44                   ; $8AB9E4
    stz.b $42                   ; $8AB9E6
CODE_8AB9E8:
    ldy.w #$0010                ; $8AB9E8
    ldx.b $42                   ; $8AB9EB
CODE_8AB9ED:
    lda.l $7F0000,x             ; $8AB9ED
    sta.w $2118                 ; $8AB9F1
    inx                         ; $8AB9F4
    inx                         ; $8AB9F5
    dey                         ; $8AB9F6
    bne CODE_8AB9ED             ; $8AB9F7
    ldy.w #$0010                ; $8AB9F9
    ldx.b $42                   ; $8AB9FC
CODE_8AB9FE:
    lda.l $7F0000,x             ; $8AB9FE
    ora.w #$0400                ; $8ABA02
    sta.w $2118                 ; $8ABA05
    inx                         ; $8ABA08
    inx                         ; $8ABA09
    dey                         ; $8ABA0A
    bne CODE_8AB9FE             ; $8ABA0B
    lda.b $42                   ; $8ABA0D
    clc                         ; $8ABA0F
    adc.w #$0020                ; $8ABA10
    sta.b $42                   ; $8ABA13
    dec.b $44                   ; $8ABA15
    bne CODE_8AB9E8             ; $8ABA17
    lda.w #$000E                ; $8ABA19
    sta.b $44                   ; $8ABA1C
    stz.b $42                   ; $8ABA1E
CODE_8ABA20:
    ldy.w #$0010                ; $8ABA20
    ldx.b $42                   ; $8ABA23
CODE_8ABA25:
    lda.l $7F0000,x             ; $8ABA25
    ora.w #$0800                ; $8ABA29
    sta.w $2118                 ; $8ABA2C
    inx                         ; $8ABA2F
    inx                         ; $8ABA30
    dey                         ; $8ABA31
    bne CODE_8ABA25             ; $8ABA32
    ldy.w #$0010                ; $8ABA34
    ldx.b $42                   ; $8ABA37
CODE_8ABA39:
    lda.l $7F0000,x             ; $8ABA39
    ora.w #$0C00                ; $8ABA3D
    sta.w $2118                 ; $8ABA40
    inx                         ; $8ABA43
    inx                         ; $8ABA44
    dey                         ; $8ABA45
    bne CODE_8ABA39             ; $8ABA46
    lda.b $42                   ; $8ABA48
    clc                         ; $8ABA4A
    adc.w #$0020                ; $8ABA4B
    sta.b $42                   ; $8ABA4E
    dec.b $44                   ; $8ABA50
    bne CODE_8ABA20             ; $8ABA52
    rts                         ; $8ABA54
func_8ABA55:
    ldx.w #$EE52                ; $8ABA55
    ldy.w #$4000                ; $8ABA58
    lda.w #$0096                ; $8ABA5B
    jsl.l decompress_to_vram_long; $8ABA5E
    ldx.w #$FA8B                ; $8ABA62
    ldy.w #$7000                ; $8ABA65
    lda.w #$0085                ; $8ABA68
    jsl.l decompress_to_vram_long; $8ABA6B
    lda.w #$8A00                ; $8ABA6F
    ldx.w #$C658                ; $8ABA72
    ldy.w #$0200                ; $8ABA75
    jsl.l upload_cgram_now      ; $8ABA78
    rts                         ; $8ABA7C
upload_7f_to_3000:
    lda.w #$007F                ; $8ABA7D
    sta.b $14                   ; $8ABA80
    ldx.w #$0000                ; $8ABA82
    ldy.w #$3000                ; $8ABA85
    lda.w #$0800                ; $8ABA88
    jsl.l upload_vram_now       ; $8ABA8B
    rts                         ; $8ABA8F
wait_frames_long:
    clc                         ; $8ABA90
    adc.l $810024               ; $8ABA91
CODE_8ABA95:
    cmp.l $810024               ; $8ABA95
    bne CODE_8ABA95             ; $8ABA99
    rtl                         ; $8ABA9B
    incbin !baserom:$53A9C..$53ACB
text_tiles_put_0:
    lda.w #$0000                ; $8ABACB
    sta.w $2116                 ; $8ABACE
    ldx.w #$0060                ; $8ABAD1
    ldy.w #$0000                ; $8ABAD4
CODE_8ABAD7:
    lda.w $C8E4,y               ; $8ABAD7
    sta.w $2118                 ; $8ABADA
    iny                         ; $8ABADD
    iny                         ; $8ABADE
    dex                         ; $8ABADF
    bpl CODE_8ABAD7             ; $8ABAE0
    rts                         ; $8ABAE2
fade_out_wait:
    stz.w $0198                 ; $8ABAE3
    lda.w #$FF80                ; $8ABAE6
    sta.w $0196                 ; $8ABAE9
CODE_8ABAEC:
    lda.w $0196                 ; $8ABAEC
    bne CODE_8ABAEC             ; $8ABAEF
    rts                         ; $8ABAF1
fade_in_start:
    lda.w #$0F00                ; $8ABAF2
    sta.w $0198                 ; $8ABAF5
    lda.w #$0080                ; $8ABAF8
    sta.w $0196                 ; $8ABAFB
    rts                         ; $8ABAFE
password_unpack:
    ldx.w #$0000                ; $8ABAFF
CODE_8ABB02:
    lda.w $CF61,x               ; $8ABB02
    beq CODE_8ABB0F             ; $8ABB05
    inx                         ; $8ABB07
    inx                         ; $8ABB08
    inx                         ; $8ABB09
    inx                         ; $8ABB0A
    inx                         ; $8ABB0B
    inx                         ; $8ABB0C
    bra CODE_8ABB02             ; $8ABB0D
CODE_8ABB0F:
    dex                         ; $8ABB0F
    dex                         ; $8ABB10
    dex                         ; $8ABB11
    dex                         ; $8ABB12
    dex                         ; $8ABB13
    dex                         ; $8ABB14
CODE_8ABB15:
    lda.w $CF61,x               ; $8ABB15
    sta.b $42                   ; $8ABB18
    lda.w $CF63,x               ; $8ABB1A
    sta.b $44                   ; $8ABB1D
    lda.w $CF65,x               ; $8ABB1F
    xba                         ; $8ABB22
    and.w #$00FF                ; $8ABB23
    tay                         ; $8ABB26
    stz.b $46                   ; $8ABB27
    jsr.w func_8ABB5C           ; $8ABB29
    eor.w $BC3D,y               ; $8ABB2C
    and.b $46                   ; $8ABB2F
    sta.b $48                   ; $8ABB31
    lda.w $CF65,x               ; $8ABB33
    and.w #$00FF                ; $8ABB36
    tay                         ; $8ABB39
    lda.b $48                   ; $8ABB3A
    jsr.w asl_n                 ; $8ABB3C
    sta.b $48                   ; $8ABB3F
    lda.b $46                   ; $8ABB41
    jsr.w asl_n                 ; $8ABB43
    eor.w #$FFFF                ; $8ABB46
    sta.b $46                   ; $8ABB49
    lda.b [$42]                 ; $8ABB4B
    and.b $46                   ; $8ABB4D
    ora.b $48                   ; $8ABB4F
    sta.b [$42]                 ; $8ABB51
    dex                         ; $8ABB53
    dex                         ; $8ABB54
    dex                         ; $8ABB55
    dex                         ; $8ABB56
    dex                         ; $8ABB57
    dex                         ; $8ABB58
    bpl CODE_8ABB15             ; $8ABB59
    rts                         ; $8ABB5B
func_8ABB5C:
    phy                         ; $8ABB5C
    lda.w #$0000                ; $8ABB5D
CODE_8ABB60:
    ror.w $0DAC                 ; $8ABB60
    ror.w $0DAE                 ; $8ABB63
    ror.w $0DB0                 ; $8ABB66
    ror.w $0DB2                 ; $8ABB69
    ror.w $0DB4                 ; $8ABB6C
    ror.w $0DB6                 ; $8ABB6F
    ror.w $0DB8                 ; $8ABB72
    ror.w $0DBA                 ; $8ABB75
    ror.w $0DBC                 ; $8ABB78
    ror.w $0DBE                 ; $8ABB7B
    ror.w $0DC0                 ; $8ABB7E
    ror.w $0DC2                 ; $8ABB81
    ror.w $0DC4                 ; $8ABB84
    ror.w $0DC6                 ; $8ABB87
    ror.w $0DC8                 ; $8ABB8A
    ror.w $0DCA                 ; $8ABB8D
    rol a                       ; $8ABB90
    sec                         ; $8ABB91
    rol.b $46                   ; $8ABB92
    dey                         ; $8ABB94
    bne CODE_8ABB60             ; $8ABB95
    ply                         ; $8ABB97
    rts                         ; $8ABB98
password_pack:
    jsr.w func_8ABC7F           ; $8ABB99
    stz.b $48                   ; $8ABB9C
    stz.b $92                   ; $8ABB9E
    stz.b $84                   ; $8ABBA0
    ldx.w #$0000                ; $8ABBA2
CODE_8ABBA5:
    lda.w $CF61,x               ; $8ABBA5
    beq CODE_8ABBD4             ; $8ABBA8
    sta.b $42                   ; $8ABBAA
    lda.w $CF63,x               ; $8ABBAC
    sta.b $44                   ; $8ABBAF
    lda.w $CF65,x               ; $8ABBB1
    xba                         ; $8ABBB4
    and.w #$00FF                ; $8ABBB5
    pha                         ; $8ABBB8
    lda.w $CF65,x               ; $8ABBB9
    and.w #$00FF                ; $8ABBBC
    tay                         ; $8ABBBF
    lda.b [$42]                 ; $8ABBC0
    jsr.w lsr_n                 ; $8ABBC2
    ply                         ; $8ABBC5
    eor.w $BC3D,y               ; $8ABBC6
    jsr.w func_8ABBF4           ; $8ABBC9
    inx                         ; $8ABBCC
    inx                         ; $8ABBCD
    inx                         ; $8ABBCE
    inx                         ; $8ABBCF
    inx                         ; $8ABBD0
    inx                         ; $8ABBD1
    bra CODE_8ABBA5             ; $8ABBD2
CODE_8ABBD4:
    lda.b $84                   ; $8ABBD4
    rts                         ; $8ABBD6
    incbin !baserom:$53BD7..$53BF4
func_8ABBF4:
    phy                         ; $8ABBF4
    pha                         ; $8ABBF5
CODE_8ABBF6:
    lsr a                       ; $8ABBF6
    rol.w $0DCA                 ; $8ABBF7
    rol.w $0DC8                 ; $8ABBFA
    rol.w $0DC6                 ; $8ABBFD
    rol.w $0DC4                 ; $8ABC00
    rol.w $0DC2                 ; $8ABC03
    rol.w $0DC0                 ; $8ABC06
    rol.w $0DBE                 ; $8ABC09
    rol.w $0DBC                 ; $8ABC0C
    rol.w $0DBA                 ; $8ABC0F
    rol.w $0DB8                 ; $8ABC12
    rol.w $0DB6                 ; $8ABC15
    rol.w $0DB4                 ; $8ABC18
    rol.w $0DB2                 ; $8ABC1B
    rol.w $0DB0                 ; $8ABC1E
    rol.w $0DAE                 ; $8ABC21
    rol.w $0DAC                 ; $8ABC24
    inc.b $84                   ; $8ABC27
    jsr.w func_8ABC32           ; $8ABC29
    dey                         ; $8ABC2C
    bne CODE_8ABBF6             ; $8ABC2D
    pla                         ; $8ABC2F
    ply                         ; $8ABC30
    rts                         ; $8ABC31
func_8ABC32:
    pha                         ; $8ABC32
    lda.b $92                   ; $8ABC33
    clc                         ; $8ABC35
    adc.w $0DCA                 ; $8ABC36
    sta.b $92                   ; $8ABC39
    pla                         ; $8ABC3B
    rts                         ; $8ABC3C
    incbin !baserom:$53C3D..$53C7F
func_8ABC7F:
    ldx.w #$001E                ; $8ABC7F
CODE_8ABC82:
    stz.w $0DAC,x               ; $8ABC82
    dex                         ; $8ABC85
    dex                         ; $8ABC86
    bpl CODE_8ABC82             ; $8ABC87
    rts                         ; $8ABC89
password_encode:
    ldy.w #$0000                ; $8ABC8A
CODE_8ABC8D:
    iny                         ; $8ABC8D
    sec                         ; $8ABC8E
    sbc.w #$0005                ; $8ABC8F
    bpl CODE_8ABC8D             ; $8ABC92
    dey                         ; $8ABC94
    tya                         ; $8ABC95
    sta.b $42                   ; $8ABC96
    ldx.w #$0000                ; $8ABC98
CODE_8ABC9B:
    ldy.w #$0005                ; $8ABC9B
    jsr.w func_8ABB5C           ; $8ABC9E
    tay                         ; $8ABCA1
    sep.b #$20                  ; $8ABCA2
    lda.w $BC5F,y               ; $8ABCA4
    sta.w $0D40,x               ; $8ABCA7
    rep.b #$20                  ; $8ABCAA
    inx                         ; $8ABCAC
    dec.b $42                   ; $8ABCAD
    bpl CODE_8ABC9B             ; $8ABCAF
    sep.b #$20                  ; $8ABCB1
    stz.w $0D40,x               ; $8ABCB3
    stz.w $0D41,x               ; $8ABCB6
    rep.b #$20                  ; $8ABCB9
    lda.b $92                   ; $8ABCBB
    and.w #$001F                ; $8ABCBD
    tay                         ; $8ABCC0
    lda.w $BC5F,y               ; $8ABCC1
    and.w #$00FF                ; $8ABCC4
    ora.w $0D40,x               ; $8ABCC7
    sta.w $0D40,x               ; $8ABCCA
    inx                         ; $8ABCCD
    lda.b $92                   ; $8ABCCE
    and.w #$03E0                ; $8ABCD0
    lsr a                       ; $8ABCD3
    lsr a                       ; $8ABCD4
    lsr a                       ; $8ABCD5
    lsr a                       ; $8ABCD6
    lsr a                       ; $8ABCD7
    tay                         ; $8ABCD8
    lda.w $BC5F,y               ; $8ABCD9
    and.w #$00FF                ; $8ABCDC
    ora.w $0D40,x               ; $8ABCDF
    sta.w $0D40,x               ; $8ABCE2
    inx                         ; $8ABCE5
    stz.w $0D40,x               ; $8ABCE6
    rts                         ; $8ABCE9
password_decode:
    lda.w #$0000                ; $8ABCEA
    sta.b $48                   ; $8ABCED
    stz.b $92                   ; $8ABCEF
    jsr.w func_8ABC7F           ; $8ABCF1
    stz.b $84                   ; $8ABCF4
    sep.b #$20                  ; $8ABCF6
    ldx.w #$0000                ; $8ABCF8
CODE_8ABCFB:
    lda.w $0D40,x               ; $8ABCFB
    beq CODE_8ABD03             ; $8ABCFE
    inx                         ; $8ABD00
    bra CODE_8ABCFB             ; $8ABD01
CODE_8ABD03:
    dex                         ; $8ABD03
    dex                         ; $8ABD04
    dex                         ; $8ABD05
CODE_8ABD06:
    sep.b #$20                  ; $8ABD06
    lda.w $0D40,x               ; $8ABD08
    ldy.w #$0000                ; $8ABD0B
CODE_8ABD0E:
    cmp.w $BC5F,y               ; $8ABD0E
    beq CODE_8ABD16             ; $8ABD11
    iny                         ; $8ABD13
    bra CODE_8ABD0E             ; $8ABD14
CODE_8ABD16:
    rep.b #$20                  ; $8ABD16
    tya                         ; $8ABD18
    ldy.w #$0005                ; $8ABD19
    jsr.w func_8ABBF4           ; $8ABD1C
    dex                         ; $8ABD1F
    bpl CODE_8ABD06             ; $8ABD20
    rep.b #$20                  ; $8ABD22
    rts                         ; $8ABD24
password_checksum_ok:
    lda.b $92                   ; $8ABD25
    and.w #$03FF                ; $8ABD27
    sta.b $92                   ; $8ABD2A
    ldx.w #$0000                ; $8ABD2C
CODE_8ABD2F:
    lda.w $0D42,x               ; $8ABD2F
    and.w #$00FF                ; $8ABD32
    beq CODE_8ABD3A             ; $8ABD35
    inx                         ; $8ABD37
    bra CODE_8ABD2F             ; $8ABD38
CODE_8ABD3A:
    sep.b #$20                  ; $8ABD3A
    lda.w $0D40,x               ; $8ABD3C
    ldy.w #$0000                ; $8ABD3F
CODE_8ABD42:
    cmp.w $BC5F,y               ; $8ABD42
    beq CODE_8ABD4A             ; $8ABD45
    iny                         ; $8ABD47
    bra CODE_8ABD42             ; $8ABD48
CODE_8ABD4A:
    sty.b $42                   ; $8ABD4A
    lda.w $0D41,x               ; $8ABD4C
    ldy.w #$0000                ; $8ABD4F
CODE_8ABD52:
    cmp.w $BC5F,y               ; $8ABD52
    beq CODE_8ABD5A             ; $8ABD55
    iny                         ; $8ABD57
    bra CODE_8ABD52             ; $8ABD58
CODE_8ABD5A:
    rep.b #$20                  ; $8ABD5A
    tya                         ; $8ABD5C
    asl a                       ; $8ABD5D
    asl a                       ; $8ABD5E
    asl a                       ; $8ABD5F
    asl a                       ; $8ABD60
    asl a                       ; $8ABD61
    ora.b $42                   ; $8ABD62
    sta.b $42                   ; $8ABD64
    cmp.b $92                   ; $8ABD66
    beq CODE_8ABD6C             ; $8ABD68
    sec                         ; $8ABD6A
    rts                         ; $8ABD6B
CODE_8ABD6C:
    clc                         ; $8ABD6C
    rts                         ; $8ABD6D
    incbin !baserom:$53D6E..$53E2E
password_show_open:
    lda.w $0008                 ; $8ABE2E
    pha                         ; $8ABE31
    stz.w $0196                 ; $8ABE32
    stz.w $0198                 ; $8ABE35
    stz.w $019A                 ; $8ABE38
    lda.w #$8934                ; $8ABE3B
    sta.w $0008                 ; $8ABE3E
    jsl.l clear_dp_scratch      ; $8ABE41
    stz.b $8E                   ; $8ABE45
    ldx.w #$C4B0                ; $8ABE47
    lda.w #$008A                ; $8ABE4A
    jsl.l video_load_screen_long; $8ABE4D
    jsl.l clear_vram            ; $8ABE51
    jsr.w fade_out_wait         ; $8ABE55
    jsr.w password_hdma_off     ; $8ABE58
    jsr.w wipe_columns          ; $8ABE5B
password_show_graphics:
    stz.b $8E                   ; $8ABE5E
    ldx.w #$8601                ; $8ABE60
    ldy.w #$1000                ; $8ABE63
    lda.w #$0088                ; $8ABE66
    jsl.l decompress_to_vram_long; $8ABE69
    jsr.w func_8ABA55           ; $8ABE6D
    lda.w #$8560                ; $8ABE70
    ldx.w #$FA37                ; $8ABE73
    ldy.w #$0040                ; $8ABE76
    jsl.l upload_cgram_now      ; $8ABE79
    jsr.w wipe_columns          ; $8ABE7D
    jsr.w fade_in_start         ; $8ABE80
    jsr.w wait_one_frame        ; $8ABE83
password_show_text:
    inc.b $8E                   ; $8ABE86
    inc.w $0D7C                 ; $8ABE88
    jsl.l func_8A8EEF           ; $8ABE8B
    jsr.w clear_tilemap_7f      ; $8ABE8F
    jsr.w func_8ABF4E           ; $8ABE92
    lda.w #$FC5F                ; $8ABE95
    sta.w $0008                 ; $8ABE98
    inc.b $8E                   ; $8ABE9B
    jsr.w password_pack         ; $8ABE9D
    jsr.w password_encode       ; $8ABEA0
    jsr.w func_8ABF76           ; $8ABEA3
    ldx.w #$0000                ; $8ABEA6
    ldy.w #$001F                ; $8ABEA9
    stx.b $3E                   ; $8ABEAC
    sty.b $40                   ; $8ABEAE
    lda.w #$0010                ; $8ABEB0
    sta.b $3C                   ; $8ABEB3
    stz.b $68                   ; $8ABEB5
    lda.w #$0001                ; $8ABEB7
    sta.b $86                   ; $8ABEBA
    jsr.w string_space_fours    ; $8ABEBC
    lda.w #$0D60                ; $8ABEBF
    jsr.w text_print_centered   ; $8ABEC2
    lda.w #$0008                ; $8ABEC5
    jsl.l wait_frames_long      ; $8ABEC8
password_show_wait:
    lda.w #$0F00                ; $8ABECC
    sta.w $0198                 ; $8ABECF
    lda.w #$0080                ; $8ABED2
    sta.w $0196                 ; $8ABED5
CODE_8ABED8:
    lda.w #$C0C0                ; $8ABED8
    trb.w $000E                 ; $8ABEDB
    bne CODE_8ABEFA             ; $8ABEDE
    lda.w #$C0C0                ; $8ABEE0
    trb.w $0012                 ; $8ABEE3
    bne CODE_8ABEFA             ; $8ABEE6
    lda.w #$C0C0                ; $8ABEE8
    trb.w $0016                 ; $8ABEEB
    bne CODE_8ABEFA             ; $8ABEEE
    lda.w #$C0C0                ; $8ABEF0
    trb.w $001A                 ; $8ABEF3
    bne CODE_8ABEFA             ; $8ABEF6
    bra CODE_8ABED8             ; $8ABEF8
CODE_8ABEFA:
    stz.w $0198                 ; $8ABEFA
    lda.w #$FFC0                ; $8ABEFD
    sta.w $0196                 ; $8ABF00
CODE_8ABF03:
    lda.w $019A                 ; $8ABF03
    bne CODE_8ABF03             ; $8ABF06
    pla                         ; $8ABF08
    sta.w $0008                 ; $8ABF09
    rts                         ; $8ABF0C
    incbin !baserom:$53F0D..$53F2A
string_space_fours:
    sep.b #$20                  ; $8ABF2A
    ldy.w #$0000                ; $8ABF2C
    ldx.w #$0000                ; $8ABF2F
CODE_8ABF32:
    lda.w $0D40,x               ; $8ABF32
    sta.w $0D60,y               ; $8ABF35
    inx                         ; $8ABF38
    iny                         ; $8ABF39
    pha                         ; $8ABF3A
    txa                         ; $8ABF3B
    and.b #$03                  ; $8ABF3C
    bne CODE_8ABF46             ; $8ABF3E
    lda.b #$20                  ; $8ABF40
    sta.w $0D60,y               ; $8ABF42
    iny                         ; $8ABF45
CODE_8ABF46:
    pla                         ; $8ABF46
    cmp.b #$00                  ; $8ABF47
    bne CODE_8ABF32             ; $8ABF49
    rep.b #$20                  ; $8ABF4B
    rts                         ; $8ABF4D
func_8ABF4E:
    jsr.w func_8AC024           ; $8ABF4E
    lda.w $038E                 ; $8ABF51
    sta.b $44                   ; $8ABF54
    lda.w $0390                 ; $8ABF56
    sta.b $46                   ; $8ABF59
    jsr.w bcd_to_binary         ; $8ABF5B
    lda.b $42                   ; $8ABF5E
    sta.w $038E                 ; $8ABF60
    lda.w $03BD                 ; $8ABF63
    sta.b $44                   ; $8ABF66
    lda.w $03BF                 ; $8ABF68
    sta.b $46                   ; $8ABF6B
    jsr.w bcd_to_binary         ; $8ABF6D
    lda.b $42                   ; $8ABF70
    sta.w $03BD                 ; $8ABF72
    rts                         ; $8ABF75
func_8ABF76:
    lda.w $038E                 ; $8ABF76
    sta.b $42                   ; $8ABF79
    jsr.w binary_to_bcd5        ; $8ABF7B
    lda.b $44                   ; $8ABF7E
    sta.w $038E                 ; $8ABF80
    lda.b $46                   ; $8ABF83
    sta.w $0390                 ; $8ABF85
    lda.w $03BD                 ; $8ABF88
    sta.b $42                   ; $8ABF8B
    jsr.w binary_to_bcd5        ; $8ABF8D
    lda.b $44                   ; $8ABF90
    sta.w $03BD                 ; $8ABF92
    lda.b $46                   ; $8ABF95
    sta.w $03BF                 ; $8ABF97
    jsr.w func_8AC03D           ; $8ABF9A
    rts                         ; $8ABF9D
bcd_to_binary:
    stz.b $42                   ; $8ABF9E
    ldx.w #$0000                ; $8ABFA0
CODE_8ABFA3:
    jsr.w func_8ABFBC           ; $8ABFA3
    tay                         ; $8ABFA6
CODE_8ABFA7:
    beq CODE_8ABFB4             ; $8ABFA7
    clc                         ; $8ABFA9
    lda.b $42                   ; $8ABFAA
    adc.w $CABA,x               ; $8ABFAC
    sta.b $42                   ; $8ABFAF
    dey                         ; $8ABFB1
    bra CODE_8ABFA7             ; $8ABFB2
CODE_8ABFB4:
    inx                         ; $8ABFB4
    inx                         ; $8ABFB5
    cpx.w #$000A                ; $8ABFB6
    bne CODE_8ABFA3             ; $8ABFB9
    rts                         ; $8ABFBB
func_8ABFBC:
    phy                         ; $8ABFBC
    ldy.w #$0004                ; $8ABFBD
CODE_8ABFC0:
    lsr.b $46                   ; $8ABFC0
    ror.b $44                   ; $8ABFC2
    ror a                       ; $8ABFC4
    dey                         ; $8ABFC5
    bne CODE_8ABFC0             ; $8ABFC6
    ldy.w #$000C                ; $8ABFC8
    jsr.w lsr_n                 ; $8ABFCB
    ply                         ; $8ABFCE
    rts                         ; $8ABFCF
binary_to_bcd5:
    ldx.w #$0008                ; $8ABFD0
    stz.b $44                   ; $8ABFD3
    stz.b $46                   ; $8ABFD5
CODE_8ABFD7:
    lda.b $42                   ; $8ABFD7
    sec                         ; $8ABFD9
    sbc.w $CABA,x               ; $8ABFDA
    bcc CODE_8ABFE5             ; $8ABFDD
    sta.b $42                   ; $8ABFDF
    inc.b $44                   ; $8ABFE1
    bra CODE_8ABFD7             ; $8ABFE3
CODE_8ABFE5:
    asl.b $44                   ; $8ABFE5
    rol.b $46                   ; $8ABFE7
    asl.b $44                   ; $8ABFE9
    rol.b $46                   ; $8ABFEB
    asl.b $44                   ; $8ABFED
    rol.b $46                   ; $8ABFEF
    asl.b $44                   ; $8ABFF1
    rol.b $46                   ; $8ABFF3
    dex                         ; $8ABFF5
    dex                         ; $8ABFF6
    bpl CODE_8ABFD7             ; $8ABFF7
    ldy.w #$0004                ; $8ABFF9
CODE_8ABFFC:
    lsr.b $46                   ; $8ABFFC
    ror.b $44                   ; $8ABFFE
    dey                         ; $8AC000
    bne CODE_8ABFFC             ; $8AC001
    rts                         ; $8AC003
    incbin !baserom:$54004..$54013
password_code_infodata:
    lda.w #$0004                ; $8AC013
    bra CODE_8AC020             ; $8AC016
    incbin !baserom:$54018..$54020
CODE_8AC020:
    tsb.w $03E2                 ; $8AC020
    rts                         ; $8AC023
func_8AC024:
    ldy.w #$000C                ; $8AC024
CODE_8AC027:
    lsr.w $0390                 ; $8AC027
    ror.w $038E                 ; $8AC02A
    dey                         ; $8AC02D
    bne CODE_8AC027             ; $8AC02E
    ldy.w #$000C                ; $8AC030
CODE_8AC033:
    lsr.w $03BF                 ; $8AC033
    ror.w $03BD                 ; $8AC036
    dey                         ; $8AC039
    bne CODE_8AC033             ; $8AC03A
    rts                         ; $8AC03C
func_8AC03D:
    ldy.w #$000C                ; $8AC03D
CODE_8AC040:
    asl.w $038E                 ; $8AC040
    rol.w $0390                 ; $8AC043
    dey                         ; $8AC046
    bne CODE_8AC040             ; $8AC047
    ldy.w #$000C                ; $8AC049
CODE_8AC04C:
    asl.w $03BD                 ; $8AC04C
    rol.w $03BF                 ; $8AC04F
    dey                         ; $8AC052
    bne CODE_8AC04C             ; $8AC053
    rts                         ; $8AC055
title_card_show:
    lda.w $0008                 ; $8AC056
    pha                         ; $8AC059
    stz.w $0196                 ; $8AC05A
    stz.w $0198                 ; $8AC05D
    stz.w $019A                 ; $8AC060
    jsl.l clear_dp_scratch      ; $8AC063
    stz.b $8E                   ; $8AC067
    ldx.w #$C4B0                ; $8AC069
    lda.w #$008A                ; $8AC06C
    jsl.l video_load_screen_long; $8AC06F
    jsl.l clear_vram            ; $8AC073
    jsr.w func_8ABA55           ; $8AC077
    ldy.w #$0800                ; $8AC07A
    ldx.w #$F861                ; $8AC07D
    lda.w #$0087                ; $8AC080
    jsl.l decompress_to_vram_long; $8AC083
    jsr.w text_palettes_load    ; $8AC087
    ldy.w #$0000                ; $8AC08A
    ldx.w #$F5BC                ; $8AC08D
    lda.w #$0087                ; $8AC090
    jsl.l decompress_to_vram_long; $8AC093
    jsl.l func_8A8EEF           ; $8AC097
    jsr.w clear_tilemap_7f      ; $8AC09B
    lda.w #$FC5F                ; $8AC09E
    sta.w $0008                 ; $8AC0A1
    inc.b $8E                   ; $8AC0A4
    sep.b #$20                  ; $8AC0A6
    lda.b #$BF                  ; $8AC0A8
    sta.w $210E                 ; $8AC0AA
    lda.b #$FF                  ; $8AC0AD
    sta.w $210E                 ; $8AC0AF
    rep.b #$20                  ; $8AC0B2
    lda.w #$0058                ; $8AC0B4
    sta.b $42                   ; $8AC0B7
    lda.w #$0014                ; $8AC0B9
    sta.b $44                   ; $8AC0BC
    jsr.w sprite_slots_six      ; $8AC0BE
    jsr.w func_8AC0F2           ; $8AC0C1
    lda.w #$0F00                ; $8AC0C4
    sta.w $0198                 ; $8AC0C7
    lda.w #$0080                ; $8AC0CA
    sta.w $0196                 ; $8AC0CD
    ldx.w #$00B4                ; $8AC0D0
CODE_8AC0D3:
    phx                         ; $8AC0D3
    lda.w #$0001                ; $8AC0D4
    jsl.l wait_frames_long      ; $8AC0D7
    plx                         ; $8AC0DB
    dex                         ; $8AC0DC
    bne CODE_8AC0D3             ; $8AC0DD
    stz.w $0198                 ; $8AC0DF
    lda.w #$FFC0                ; $8AC0E2
    sta.w $0196                 ; $8AC0E5
CODE_8AC0E8:
    lda.w $019A                 ; $8AC0E8
    bne CODE_8AC0E8             ; $8AC0EB
    pla                         ; $8AC0ED
    sta.w $0008                 ; $8AC0EE
    rts                         ; $8AC0F1
func_8AC0F2:
    stz.b $90                   ; $8AC0F2
    inc.b $90                   ; $8AC0F4
    ldx.w #$000C                ; $8AC0F6
    ldy.w #$0006                ; $8AC0F9
    lda.w #$C44F                ; $8AC0FC
    jsr.w text_print_tiles_at   ; $8AC0FF
    dec.b $90                   ; $8AC102
    ldx.w #$0006                ; $8AC104
    ldy.w #$0016                ; $8AC107
    lda.w #$C458                ; $8AC10A
    jsr.w text_print_tiles_at   ; $8AC10D
    ldx.w #$0005                ; $8AC110
    ldy.w #$0017                ; $8AC113
    lda.w #$C46D                ; $8AC116
    jsr.w text_print_tiles_at   ; $8AC119
    ldx.w #$0005                ; $8AC11C
    ldy.w #$0018                ; $8AC11F
    lda.w #$C484                ; $8AC122
    jsr.w text_print_tiles_at   ; $8AC125
    inc.b $90                   ; $8AC128
    ldx.w #$0006                ; $8AC12A
    ldy.w #$001A                ; $8AC12D
    lda.w #$C49B                ; $8AC130
    jsr.w text_print_tiles_at   ; $8AC133
    rts                         ; $8AC136
text_print_tiles_at:
    stx.b $3A                   ; $8AC137
    sty.b $3C                   ; $8AC139
    jsr.w text_print_tiles      ; $8AC13B
    rts                         ; $8AC13E
text_print_tiles:
    sta.b $48                   ; $8AC13F
    ldy.w #$0000                ; $8AC141
CODE_8AC144:
    lda.b ($48),y               ; $8AC144
    and.w #$00FF                ; $8AC146
    beq CODE_8AC156             ; $8AC149
    cmp.w #$0080                ; $8AC14B
    db $F0,$07 ; beq            ; $8AC14E
    jsr.w text_put_tile_char    ; $8AC150
    iny                         ; $8AC153
    bra CODE_8AC144             ; $8AC154
CODE_8AC156:
    rts                         ; $8AC156
    incbin !baserom:$54157..$5415D
text_buffer_screen:
    ldx.w #$0000                ; $8AC15D
CODE_8AC160:
    lda.l $7F7D00,x             ; $8AC160
    and.w #$00FF                ; $8AC164
    beq CODE_8AC174             ; $8AC167
    cmp.w #$0080                ; $8AC169
    beq CODE_8AC183             ; $8AC16C
    jsr.w text_put_tile_char    ; $8AC16E
CODE_8AC171:
    inx                         ; $8AC171
    bra CODE_8AC160             ; $8AC172
CODE_8AC174:
    ldx.w #$C224                ; $8AC174
    bit.w $1B37                 ; $8AC177
    bpl CODE_8AC17F             ; $8AC17A
    incbin !baserom:$5417C..$5417F
CODE_8AC17F:
    jsr.w text_screen_init      ; $8AC17F
    rts                         ; $8AC182
CODE_8AC183:
    inc.b $3C                   ; $8AC183
    lda.w #$0002                ; $8AC185
    sta.b $3A                   ; $8AC188
    lda.b $3C                   ; $8AC18A
    cmp.w #$000C                ; $8AC18C
    bmi CODE_8AC19A             ; $8AC18F
    stz.b $90                   ; $8AC191
    cmp.w #$000C                ; $8AC193
    bne CODE_8AC19A             ; $8AC196
    inc.b $3C                   ; $8AC198
CODE_8AC19A:
    bra CODE_8AC171             ; $8AC19A
sprite_slots_six:
    ldx.w #$0000                ; $8AC19C
CODE_8AC19F:
    lda.w $C443,x               ; $8AC19F
    ora.w #$2000                ; $8AC1A2
    sta.w $0800,x               ; $8AC1A5
    lda.b $42                   ; $8AC1A8
    sta.w $0600,x               ; $8AC1AA
    clc                         ; $8AC1AD
    adc.w #$0010                ; $8AC1AE
    sta.b $42                   ; $8AC1B1
    lda.b $44                   ; $8AC1B3
    sta.w $0700,x               ; $8AC1B5
    lda.w #$0001                ; $8AC1B8
    sta.w $0500,x               ; $8AC1BB
    inx                         ; $8AC1BE
    inx                         ; $8AC1BF
    cpx.w #$000C                ; $8AC1C0
    bne CODE_8AC19F             ; $8AC1C3
    rts                         ; $8AC1C5
text_screen_init:
    phx                         ; $8AC1C6
    inc.w $0D7C                 ; $8AC1C7
    ldx.w #$0000                ; $8AC1CA
    ldy.w #$001F                ; $8AC1CD
    stx.b $3E                   ; $8AC1D0
    sty.b $40                   ; $8AC1D2
    stz.b $68                   ; $8AC1D4
    lda.w #$0001                ; $8AC1D6
    sta.b $86                   ; $8AC1D9
    lda.w #$0003                ; $8AC1DB
    sta.b $3C                   ; $8AC1DE
    lda.w #$1000                ; $8AC1E0
    sta.w $2116                 ; $8AC1E3
    ldx.w #$0060                ; $8AC1E6
    ldy.w #$0000                ; $8AC1E9
CODE_8AC1EC:
    lda.w $C8E4,y               ; $8AC1EC
    sta.w $2118                 ; $8AC1EF
    iny                         ; $8AC1F2
    iny                         ; $8AC1F3
    dex                         ; $8AC1F4
    bpl CODE_8AC1EC             ; $8AC1F5
    ldy.w #$001C                ; $8AC1F7
    ldx.w #$0184                ; $8AC1FA
    lda.w #$0502                ; $8AC1FD
CODE_8AC200:
    sta.l $7F0000,x             ; $8AC200
    inx                         ; $8AC204
    inx                         ; $8AC205
    dey                         ; $8AC206
    bne CODE_8AC200             ; $8AC207
    pla                         ; $8AC209
    jsr.w text_print_centered   ; $8AC20A
    ldy.w #$0080                ; $8AC20D
    ldx.w #$0080                ; $8AC210
CODE_8AC213:
    lda.l $7F0000,x             ; $8AC213
    and.w #$E3FF                ; $8AC217
    sta.l $7F0000,x             ; $8AC21A
    inx                         ; $8AC21E
    inx                         ; $8AC21F
    dey                         ; $8AC220
    bne CODE_8AC213             ; $8AC221
    rts                         ; $8AC223
    incbin !baserom:$54224..$5424B
text_palettes_load:
    sep.b #$20                  ; $8AC24B
    lda.b #$20                  ; $8AC24D
    sta.w $2121                 ; $8AC24F
    ldy.w #$0060                ; $8AC252
CODE_8AC255:
    stz.w $2122                 ; $8AC255
    dey                         ; $8AC258
    bne CODE_8AC255             ; $8AC259
    rep.b #$20                  ; $8AC25B
    lda.w #$8A80                ; $8AC25D
    ldx.w #$C964                ; $8AC260
    ldy.w #$0020                ; $8AC263
    jsl.l upload_cgram_now      ; $8AC266
    lda.w #$8A60                ; $8AC26A
    ldx.w #$C984                ; $8AC26D
    ldy.w #$0040                ; $8AC270
    jsl.l upload_cgram_now      ; $8AC273
    rts                         ; $8AC277
text_put_tile_char:
    php                         ; $8AC278
    phy                         ; $8AC279
    phx                         ; $8AC27A
    rep.b #$20                  ; $8AC27B
    and.w #$00FF                ; $8AC27D
    ldy.b $3A                   ; $8AC280
    sty.b $36                   ; $8AC282
    ldy.b $3C                   ; $8AC284
    sty.b $38                   ; $8AC286
    jsr.w tilemap_offset_c2cc   ; $8AC288
    jsr.w char_to_tile_index    ; $8AC28B
    ldy.b $90                   ; $8AC28E
    beq CODE_8AC295             ; $8AC290
    ora.w #$0400                ; $8AC292
CODE_8AC295:
    ora.w #$3880                ; $8AC295
    sta.l $7F0000,x             ; $8AC298
    inc.b $3A                   ; $8AC29C
    plx                         ; $8AC29E
    ply                         ; $8AC29F
    plp                         ; $8AC2A0
    rts                         ; $8AC2A1
char_to_tile_index:
    cmp.w #$0020                ; $8AC2A2
    beq CODE_8AC2BB             ; $8AC2A5
    cmp.w #$002E                ; $8AC2A7
    beq CODE_8AC2BF             ; $8AC2AA
    cmp.w #$0021                ; $8AC2AC
    db $F0,$12 ; beq            ; $8AC2AF
    cmp.w #$0041                ; $8AC2B1
    bmi CODE_8AC2C7             ; $8AC2B4
    sec                         ; $8AC2B6
    sbc.w #$0041                ; $8AC2B7
    rts                         ; $8AC2BA
CODE_8AC2BB:
    lda.w #$0025                ; $8AC2BB
    rts                         ; $8AC2BE
CODE_8AC2BF:
    lda.w #$0024                ; $8AC2BF
    rts                         ; $8AC2C2
    incbin !baserom:$542C3..$542C7
CODE_8AC2C7:
    sec                         ; $8AC2C7
    sbc.w #$0016                ; $8AC2C8
    rts                         ; $8AC2CB
tilemap_offset_c2cc:
    pha                         ; $8AC2CC
    lda.b $38                   ; $8AC2CD
    asl a                       ; $8AC2CF
    asl a                       ; $8AC2D0
    asl a                       ; $8AC2D1
    asl a                       ; $8AC2D2
    asl a                       ; $8AC2D3
    asl a                       ; $8AC2D4
    asl.b $36                   ; $8AC2D5
    clc                         ; $8AC2D7
    adc.b $36                   ; $8AC2D8
    tax                         ; $8AC2DA
    pla                         ; $8AC2DB
    rts                         ; $8AC2DC
track_snap:
    lda.w $03DC                 ; $8AC2DD
    and.w #$0300                ; $8AC2E0
    beq CODE_8AC2EF             ; $8AC2E3
    cmp.w #$0100                ; $8AC2E5
    beq CODE_8AC311             ; $8AC2E8
    cmp.w #$0200                ; $8AC2EA
    beq CODE_8AC333             ; $8AC2ED
CODE_8AC2EF:
    ldx.w #$0000                ; $8AC2EF static
CODE_8AC2F2:
    lda.l $8ACAC4,x             ; $8AC2F2 static
    and.w #$00FF                ; $8AC2F6 static
    cmp.w $0444                 ; $8AC2F9 static
    beq CODE_8AC306             ; $8AC2FC static
    bpl CODE_8AC306             ; $8AC2FE static
    inx                         ; $8AC300 static
    jsr.w money_skip_bonus      ; $8AC301 static
    bra CODE_8AC2F2             ; $8AC304 static
CODE_8AC306:
    lda.l $8ACAC4,x             ; $8AC306 static
    and.w #$00FF                ; $8AC30A static
    sta.w $0444                 ; $8AC30D static
    rtl                         ; $8AC310 static
CODE_8AC311:
    ldx.w #$0000                ; $8AC311
CODE_8AC314:
    lda.l $8ACADE,x             ; $8AC314
    and.w #$00FF                ; $8AC318
    cmp.w $0444                 ; $8AC31B
    beq CODE_8AC328             ; $8AC31E
    bpl CODE_8AC328             ; $8AC320
    inx                         ; $8AC322
    jsr.w money_skip_bonus      ; $8AC323
    bra CODE_8AC314             ; $8AC326
CODE_8AC328:
    lda.l $8ACADE,x             ; $8AC328
    and.w #$00FF                ; $8AC32C
    sta.w $0444                 ; $8AC32F
    rtl                         ; $8AC332
CODE_8AC333:
    ldx.w #$0000                ; $8AC333
CODE_8AC336:
    lda.l $8ACB01,x             ; $8AC336
    and.w #$00FF                ; $8AC33A
    cmp.w $0444                 ; $8AC33D
    beq CODE_8AC34A             ; $8AC340
    bpl CODE_8AC34A             ; $8AC342 static
    inx                         ; $8AC344 static
    jsr.w money_skip_bonus      ; $8AC345 static
    bra CODE_8AC336             ; $8AC348 static
CODE_8AC34A:
    lda.l $8ACB01,x             ; $8AC34A
    and.w #$00FF                ; $8AC34E
    sta.w $0444                 ; $8AC351
    rtl                         ; $8AC354
money_skip_bonus:
    lda.w $03DC                 ; $8AC355
    bit.w #$1000                ; $8AC358
    beq CODE_8AC378             ; $8AC35B
    sed                         ; $8AC35D static
    lda.w $038E                 ; $8AC35E static
    clc                         ; $8AC361 static
    adc.w #$5000                ; $8AC362 static
    sta.w $038E                 ; $8AC365 static
    sta.w $03BD                 ; $8AC368 static
    lda.w $0390                 ; $8AC36B static
    adc.w #$0007                ; $8AC36E static
    sta.w $0390                 ; $8AC371 static
    sta.w $03BF                 ; $8AC374 static
    cld                         ; $8AC377 static
CODE_8AC378:
    rts                         ; $8AC378
    incbin !baserom:$54379..$58000
