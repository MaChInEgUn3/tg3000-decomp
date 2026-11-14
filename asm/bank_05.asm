; bank $85 (file offset $028000)
org $858000

    tax                         ; $858000
    lda.w $8410,x               ; $858001
    and.w #$00FF                ; $858004
    tax                         ; $858007
    lda.w $8447,x               ; $858008
    asl a                       ; $85800B
    tay                         ; $85800C
    phx                         ; $85800D
    lda.w $8597,y               ; $85800E
    ldx.w $8585,y               ; $858011
    ldy.w #$0000                ; $858014
    jsl.l $82FB70               ; $858017
    plx                         ; $85801B
    phx                         ; $85801C
    lda.w $047B                 ; $85801D
    asl a                       ; $858020
    tax                         ; $858021
    jmp.w ($8025,x)             ; $858022
    incbin !baserom:$28025..$28093
    plx                         ; $858093
    lda.w #$0080                ; $858094
    trb.w $1F33                 ; $858097
    lda.w #$0420                ; $85809A
    jsr.w $817B                 ; $85809D
    lda.w #$0020                ; $8580A0
    jsr.w upload_tiles_3300     ; $8580A3
    lda.w #$0220                ; $8580A6
    jsr.w upload_tiles_3700     ; $8580A9
    lda.w #$0020                ; $8580AC
    jsr.w upload_tiles_7700     ; $8580AF
    lda.w #$0220                ; $8580B2
    jsr.w upload_tiles_7b00     ; $8580B5
    lda.w #$0000                ; $8580B8
    jsr.w $82D8                 ; $8580BB
    lda.w #$0000                ; $8580BE
    jsr.w $82FA                 ; $8580C1
    rtl                         ; $8580C4
    plx                         ; $8580C5
    lda.w #$0080                ; $8580C6
    tsb.w $1F33                 ; $8580C9
    lda.w #$0820                ; $8580CC
    jsr.w $817B                 ; $8580CF
    lda.w #$0020                ; $8580D2
    jsr.w upload_tiles_3300     ; $8580D5
    lda.w #$0220                ; $8580D8
    jsr.w upload_tiles_3700     ; $8580DB
    lda.w #$0420                ; $8580DE
    jsr.w upload_tiles_7700     ; $8580E1
    lda.w #$0620                ; $8580E4
    jsr.w upload_tiles_7b00     ; $8580E7
    lda.w #$0000                ; $8580EA
    jsr.w $82D8                 ; $8580ED
    lda.w #$0000                ; $8580F0
    jsr.w $82FA                 ; $8580F3
    rtl                         ; $8580F6
    incbin !baserom:$280F7..$2817B
    sta.b $38                   ; $85817B
    phx                         ; $85817D
    lda.w $851B,x               ; $85817E
    lsr a                       ; $858181
    lsr a                       ; $858182
    lsr a                       ; $858183
    lsr a                       ; $858184
    lsr a                       ; $858185
    sta.b $3C                   ; $858186
    lda.w $84B1,x               ; $858188
    clc                         ; $85818B
    adc.b $38                   ; $85818C
    tay                         ; $85818E
    pea.w $837F                 ; $85818F
    plb                         ; $858192
    ldx.w #$0000                ; $858193
    sep.b #$20                  ; $858196
CODE_858198:
    lda.b #$04                  ; $858198
    sta.b $3A                   ; $85819A
CODE_85819C:
    lda.w $0000,y               ; $85819C
    asl a                       ; $85819F
    rol.b $25,x                 ; $8581A0
    asl a                       ; $8581A2
    rol.b $24,x                 ; $8581A3
    asl a                       ; $8581A5
    rol.b $15,x                 ; $8581A6
    asl a                       ; $8581A8
    rol.b $14,x                 ; $8581A9
    asl a                       ; $8581AB
    rol.b $25,x                 ; $8581AC
    asl a                       ; $8581AE
    rol.b $24,x                 ; $8581AF
    asl a                       ; $8581B1
    rol.b $15,x                 ; $8581B2
    asl a                       ; $8581B4
    rol.b $14,x                 ; $8581B5
    iny                         ; $8581B7
    dec.b $3A                   ; $8581B8
    bne CODE_85819C             ; $8581BA
    inx                         ; $8581BC
    inx                         ; $8581BD
    cpx.w #$0010                ; $8581BE
    bne CODE_858198             ; $8581C1
    rep.b #$20                  ; $8581C3
    tya                         ; $8581C5
    sec                         ; $8581C6
    sbc.w #$0020                ; $8581C7
    tay                         ; $8581CA
    lda.b $14                   ; $8581CB
    sta.w $0000,y               ; $8581CD
    lda.b $16                   ; $8581D0
    sta.w $0002,y               ; $8581D2
    lda.b $18                   ; $8581D5
    sta.w $0004,y               ; $8581D7
    lda.b $1A                   ; $8581DA
    sta.w $0006,y               ; $8581DC
    lda.b $1C                   ; $8581DF
    sta.w $0008,y               ; $8581E1
    lda.b $1E                   ; $8581E4
    sta.w $000A,y               ; $8581E6
    lda.b $20                   ; $8581E9
    sta.w $000C,y               ; $8581EB
    lda.b $22                   ; $8581EE
    sta.w $000E,y               ; $8581F0
    lda.b $24                   ; $8581F3
    sta.w $0010,y               ; $8581F5
    lda.b $26                   ; $8581F8
    sta.w $0012,y               ; $8581FA
    lda.b $28                   ; $8581FD
    sta.w $0014,y               ; $8581FF
    lda.b $2A                   ; $858202
    sta.w $0016,y               ; $858204
    lda.b $2C                   ; $858207
    sta.w $0018,y               ; $858209
    lda.b $2E                   ; $85820C
    sta.w $001A,y               ; $85820E
    lda.b $30                   ; $858211
    sta.w $001C,y               ; $858213
    lda.b $32                   ; $858216
    sta.w $001E,y               ; $858218
    tya                         ; $85821B
    clc                         ; $85821C
    adc.w #$0020                ; $85821D
    tay                         ; $858220
    dec.b $3C                   ; $858221
    beq CODE_858228             ; $858223
    jmp.w $8193                 ; $858225
CODE_858228:
    plb                         ; $858228
    plx                         ; $858229
    phx                         ; $85822A
    lda.w #$007F                ; $85822B
    sta.b $14                   ; $85822E
    lda.w $851B,x               ; $858230
    pha                         ; $858233
    lda.w $84B1,x               ; $858234
    clc                         ; $858237
    adc.b $38                   ; $858238
    tax                         ; $85823A
    pla                         ; $85823B
    ldy.w #$5FE0                ; $85823C
    jsl.l upload_vram_now       ; $85823F
    plx                         ; $858243
    rts                         ; $858244
upload_tiles_3300:
    phx                         ; $858245
    sta.b $16                   ; $858246
    lda.w #$007F                ; $858248
    sta.b $14                   ; $85824B
    lda.w $84B1,x               ; $85824D
    clc                         ; $858250
    adc.b $16                   ; $858251
    tax                         ; $858253
    lda.w #$0200                ; $858254
    ldy.w #$3300                ; $858257
    jsl.l upload_vram_now       ; $85825A
    plx                         ; $85825E
    rts                         ; $85825F
upload_tiles_3700:
    phx                         ; $858260
    sta.b $16                   ; $858261
    lda.w #$007F                ; $858263
    sta.b $14                   ; $858266
    lda.w $84B1,x               ; $858268
    clc                         ; $85826B
    adc.b $16                   ; $85826C
    tax                         ; $85826E
    lda.w #$0200                ; $85826F
    ldy.w #$3700                ; $858272
    jsl.l upload_vram_now       ; $858275
    plx                         ; $858279
    rts                         ; $85827A
upload_tiles_7700:
    phx                         ; $85827B
    sta.b $16                   ; $85827C
    lda.w #$007F                ; $85827E
    sta.b $14                   ; $858281
    lda.w $84B1,x               ; $858283
    clc                         ; $858286
    adc.b $16                   ; $858287
    tax                         ; $858289
    lda.w #$0200                ; $85828A
    ldy.w #$7700                ; $85828D
    jsl.l upload_vram_now       ; $858290
    plx                         ; $858294
    rts                         ; $858295
upload_tiles_7b00:
    phx                         ; $858296
    sta.b $16                   ; $858297
    lda.w #$007F                ; $858299
    sta.b $14                   ; $85829C
    lda.w $84B1,x               ; $85829E
    clc                         ; $8582A1
    adc.b $16                   ; $8582A2
    tax                         ; $8582A4
    lda.w #$0200                ; $8582A5
    ldy.w #$7B00                ; $8582A8
    jsl.l upload_vram_now       ; $8582AB
    plx                         ; $8582AF
    rts                         ; $8582B0
    incbin !baserom:$282B1..$282D8
    phx                         ; $8582D8
    clc                         ; $8582D9
    adc.w $84B1,x               ; $8582DA
    tax                         ; $8582DD
    phb                         ; $8582DE
    pea.w $7E7E                 ; $8582DF
    plb                         ; $8582E2
    plb                         ; $8582E3
    ldy.w #$0000                ; $8582E4
CODE_8582E7:
    lda.l $7F0000,x             ; $8582E7
    sta.w $6A33,y               ; $8582EB
    inx                         ; $8582EE
    inx                         ; $8582EF
    iny                         ; $8582F0
    iny                         ; $8582F1
    cpy.w #$0020                ; $8582F2
    bne CODE_8582E7             ; $8582F5
    plb                         ; $8582F7
    plx                         ; $8582F8
    rts                         ; $8582F9
    phx                         ; $8582FA
    clc                         ; $8582FB
    adc.w $84B1,x               ; $8582FC
    tax                         ; $8582FF
    phb                         ; $858300
    pea.w $7E7E                 ; $858301
    plb                         ; $858304
    plb                         ; $858305
    ldy.w #$0000                ; $858306
CODE_858309:
    lda.l $7F0000,x             ; $858309
    sta.w $6B33,y               ; $85830D
    inx                         ; $858310
    inx                         ; $858311
    iny                         ; $858312
    iny                         ; $858313
    cpy.w #$0020                ; $858314
    bne CODE_858309             ; $858317
    plb                         ; $858319
    plx                         ; $85831A
    rts                         ; $85831B
    lda.w $047B                 ; $85831C
    asl a                       ; $85831F
    asl a                       ; $858320
    tax                         ; $858321
    lda.l $84A590,x             ; $858322
    sta.b $98                   ; $858326
    lda.l $84A592,x             ; $858328
    sta.b $9A                   ; $85832C
    txa                         ; $85832E
    lsr a                       ; $85832F
    tax                         ; $858330
    lda.l $84FF59,x             ; $858331
    sta.b $9C                   ; $858335
    lda.w $047B                 ; $858337
    asl a                       ; $85833A
    tax                         ; $85833B
    lda.l $839515,x             ; $85833C
    sta.w $1F06                 ; $858340
    lda.l $839583,x             ; $858343
    sta.w $01E4                 ; $858347
    lda.w $039C                 ; $85834A
    and.w #$00FF                ; $85834D
    beq CODE_858359             ; $858350
    incbin !baserom:$28352..$28359
CODE_858359:
    lda.l $839583,x             ; $858359
    sta.w $01E6                 ; $85835D
    lda.w $03CB                 ; $858360
    and.w #$00FF                ; $858363
    beq CODE_85836F             ; $858366
    incbin !baserom:$28368..$2836F
CODE_85836F:
    lda.l $83965F,x             ; $85836F
    sta.w $1F34                 ; $858373
    rtl                         ; $858376
    rep.b #$30                  ; $858377
    phb                         ; $858379
    pha                         ; $85837A
    phx                         ; $85837B
    phy                         ; $85837C
    phd                         ; $85837D
    phk                         ; $85837E
    plb                         ; $85837F
    lda.w $4210                 ; $858380
    lda.w #$0000                ; $858383
    tcd                         ; $858386
    jmp.w ($0008)               ; $858387
    lda.w $4210                 ; $85838A
    cli                         ; $85838D
    lda.w $0146                 ; $85838E
    beq CODE_8583AE             ; $858391
    clc                         ; $858393
    adc.w $014A                 ; $858394
    sta.w $014A                 ; $858397
    sec                         ; $85839A
    sbc.w $0148                 ; $85839B
    beq CODE_8583A5             ; $85839E
    eor.w $0146                 ; $8583A0
    bmi CODE_8583AE             ; $8583A3
CODE_8583A5:
    lda.w $0148                 ; $8583A5
    sta.w $014A                 ; $8583A8
    stz.w $0146                 ; $8583AB
CODE_8583AE:
    sep.b #$20                  ; $8583AE
    lda.w $014B                 ; $8583B0
    bne CODE_8583B7             ; $8583B3
    ora.b #$80                  ; $8583B5
CODE_8583B7:
    sta.w $2100                 ; $8583B7
    rep.b #$20                  ; $8583BA
    jsl.l $86DFE1               ; $8583BC
    jsl.l $8396CD               ; $8583C0
    inc.b $24                   ; $8583C4
    inc.b $BA                   ; $8583C6
    pld                         ; $8583C8
    ply                         ; $8583C9
    plx                         ; $8583CA
    pla                         ; $8583CB
    plb                         ; $8583CC
    rti                         ; $8583CD
    lda.w #$0E00                ; $8583CE
    sta.b $FC                   ; $8583D1
    lda.w #$0001                ; $8583D3
    trb.w $00B8                 ; $8583D6
    bne CODE_8583DE             ; $8583D9
    jmp.w $850A                 ; $8583DB
CODE_8583DE:
    lda.w $00B4                 ; $8583DE
    eor.w #$0001                ; $8583E1
    sta.w $00B4                 ; $8583E4
    beq CODE_8583EC             ; $8583E7
    jmp.w $83EC                 ; $8583E9
CODE_8583EC:
    lda.w $1E4E                 ; $8583EC
    sta.l $7E4CD0               ; $8583EF
    clc                         ; $8583F3
    adc.w #$01B0                ; $8583F4
    sta.l $7E4CD3               ; $8583F7
    lda.w $016C                 ; $8583FB
    sta.b $C8                   ; $8583FE
    lda.w $0170                 ; $858400
    sta.b $CC                   ; $858403
    lda.w $00C0                 ; $858405
    sta.w $00BE                 ; $858408
    lda.w $1E46                 ; $85840B
    sta.w $4352                 ; $85840E
    lda.w $1E48                 ; $858411
    sta.w $4362                 ; $858414
    lda.w $1E02                 ; $858417
    beq CODE_858448             ; $85841A
    sta.w $4375                 ; $85841C
    eor.w #$FFFF                ; $85841F
    sec                         ; $858422
    adc.b $FC                   ; $858423
    sta.b $FC                   ; $858425
    lda.w #$1801                ; $858427
    sta.w $4370                 ; $85842A
    lda.w $1DFE                 ; $85842D
    sta.w $4372                 ; $858430
    lda.w $1E00                 ; $858433
    sta.w $2116                 ; $858436
    sep.b #$20                  ; $858439
    lda.w $1E04                 ; $85843B
    sta.w $4374                 ; $85843E
    lda.b #$80                  ; $858441
    sta.w $420B                 ; $858443
    rep.b #$20                  ; $858446
CODE_858448:
    lda.w #$0190                ; $858448
    sta.w $4375                 ; $85844B
    lda.w #$1801                ; $85844E
    sta.w $4370                 ; $858451
    lda.w #$230C                ; $858454
    sta.w $4372                 ; $858457
    lda.w #$57A0                ; $85845A
    sta.w $2116                 ; $85845D
    sep.b #$20                  ; $858460
    lda.b #$7E                  ; $858462
    sta.w $4374                 ; $858464
    lda.b #$80                  ; $858467
    sta.w $420B                 ; $858469
    rep.b #$20                  ; $85846C
    lda.b $FC                   ; $85846E
    sec                         ; $858470
    sbc.w #$0190                ; $858471
    sta.b $FC                   ; $858474
    lda.w $1E54                 ; $858476
    beq CODE_8584A7             ; $858479
    sta.w $4375                 ; $85847B
    eor.w #$FFFF                ; $85847E
    sec                         ; $858481
    adc.b $FC                   ; $858482
    sta.b $FC                   ; $858484
    lda.w #$1801                ; $858486
    sta.w $4370                 ; $858489
    lda.w $1E52                 ; $85848C
    sta.w $4372                 ; $85848F
    lda.w #$1000                ; $858492
    sta.w $2116                 ; $858495
    sep.b #$20                  ; $858498
    lda.w $1E56                 ; $85849A
    sta.w $4374                 ; $85849D
    lda.b #$80                  ; $8584A0
    sta.w $420B                 ; $8584A2
    rep.b #$20                  ; $8584A5
CODE_8584A7:
    ldx.w $1F24                 ; $8584A7
    bmi CODE_8584D8             ; $8584AA
    lda.l $86C8AC,x             ; $8584AC
    sta.w $4372                 ; $8584B0
    lda.l $86C8AE,x             ; $8584B3
    sta.w $4375                 ; $8584B7
    eor.w #$FFFF                ; $8584BA
    sec                         ; $8584BD
    adc.b $FC                   ; $8584BE
    sta.b $FC                   ; $8584C0
    lda.w #$1380                ; $8584C2
    sta.w $2116                 ; $8584C5
    sep.b #$20                  ; $8584C8
    lda.l $86C8B0,x             ; $8584CA
    sta.w $4374                 ; $8584CE
    lda.b #$80                  ; $8584D1
    sta.w $420B                 ; $8584D3
    rep.b #$20                  ; $8584D6
CODE_8584D8:
    lda.w $0174                 ; $8584D8
    sta.b $EC                   ; $8584DB
    lda.w $0176                 ; $8584DD
    sta.b $EE                   ; $8584E0
    lda.w $017C                 ; $8584E2
    sta.b $F4                   ; $8584E5
    lda.w $017E                 ; $8584E7
    sta.b $F6                   ; $8584EA
    lda.w $01CE                 ; $8584EC
    sta.w $01CC                 ; $8584EF
    lda.w $01B6                 ; $8584F2
    sta.w $01B0                 ; $8584F5
    lda.w $01B8                 ; $8584F8
    sta.w $01B2                 ; $8584FB
    lda.w $01BA                 ; $8584FE
    sta.w $01B4                 ; $858501
    lda.w $01DC                 ; $858504
    sta.w $00E2                 ; $858507
    ldx.w #$1576                ; $85850A
    lda.w $00B4                 ; $85850D
    bne CODE_858515             ; $858510
    ldx.w #$1796                ; $858512
CODE_858515:
    stx.w $4372                 ; $858515
    lda.w #$0400                ; $858518
    sta.w $4370                 ; $85851B
    lda.w #$0220                ; $85851E
    sta.w $4375                 ; $858521
    stz.w $2102                 ; $858524
    sep.b #$20                  ; $858527
    lda.b #$81                  ; $858529
    sta.w $4374                 ; $85852B
    lda.b #$80                  ; $85852E
    sta.w $420B                 ; $858530
    lda.b #$00                  ; $858533
    sta.w $2101                 ; $858535
    rep.b #$20                  ; $858538
    stz.w $2102                 ; $85853A
    lda.w #$001E                ; $85853D
    sta.w $4375                 ; $858540
    lda.w #$2202                ; $858543
    sta.w $4370                 ; $858546
    lda.w $024A                 ; $858549
    bmi CODE_858557             ; $85854C
    sta.w $4372                 ; $85854E
    sep.b #$20                  ; $858551
    lda.b #$7E                  ; $858553
    bra CODE_85855E             ; $858555
CODE_858557:
    sta.w $4372                 ; $858557
    sep.b #$20                  ; $85855A
    lda.b #$93                  ; $85855C
CODE_85855E:
    sta.w $4374                 ; $85855E
    lda.b #$F1                  ; $858561
    sta.w $2121                 ; $858563
    lda.b #$80                  ; $858566
    sta.w $420B                 ; $858568
    rep.b #$20                  ; $85856B
    lda.w #$6A35                ; $85856D
    sta.w $4372                 ; $858570
    lda.w #$00FE                ; $858573
    sta.w $4375                 ; $858576
    sep.b #$20                  ; $858579
    lda.b #$71                  ; $85857B
    sta.w $2121                 ; $85857D
    lda.b #$7E                  ; $858580
    sta.w $4374                 ; $858582
    lda.b #$80                  ; $858585
    sta.w $420B                 ; $858587
    stz.w $2121                 ; $85858A
    lda.l $7E6A33               ; $85858D
    sta.w $2122                 ; $858591
    lda.l $7E6A34               ; $858594
    sta.w $2122                 ; $858598
    rep.b #$20                  ; $85859B
    lda.w #$2140                ; $85859D
    sta.w $024A                 ; $8585A0
    jsl.l $8396D1               ; $8585A3
    sep.b #$20                  ; $8585A7
    lda.b #$07                  ; $8585A9
    sta.w $2121                 ; $8585AB
    lda.b $A0                   ; $8585AE
    sta.w $2122                 ; $8585B0
    lda.b $A1                   ; $8585B3
    sta.w $2122                 ; $8585B5
    rep.b #$20                  ; $8585B8
    lda.w $003A                 ; $8585BA
    sep.b #$30                  ; $8585BD
    ldx.b #$21                  ; $8585BF
    stx.w $2121                 ; $8585C1
    sta.w $2122                 ; $8585C4
    xba                         ; $8585C7
    sta.w $2122                 ; $8585C8
    xba                         ; $8585CB
    ldx.b #$31                  ; $8585CC
    stx.w $2121                 ; $8585CE
    sta.w $2122                 ; $8585D1
    xba                         ; $8585D4
    sta.w $2122                 ; $8585D5
    rep.b #$30                  ; $8585D8
    lda.b $FC                   ; $8585DA
    cmp.w #$0E00                ; $8585DC
    bmi CODE_8585ED             ; $8585DF
    lda.w #$0004                ; $8585E1
    trb.w $00B8                 ; $8585E4
    beq CODE_8585ED             ; $8585E7
    jsl.l $858814               ; $8585E9
CODE_8585ED:
    lda.w #$0D43                ; $8585ED
    sta.w $4350                 ; $8585F0
    lda.w #$0F43                ; $8585F3
    sta.w $4360                 ; $8585F6
    lda.w #$2644                ; $8585F9
    sta.w $4340                 ; $8585FC
    lda.w #$4CCC                ; $8585FF
    sta.w $4342                 ; $858602
    lda.w #$2200                ; $858605
    sta.w $4310                 ; $858608
    lda.w $00C8                 ; $85860B
    sta.w $4312                 ; $85860E
    stz.w $4315                 ; $858611
    lda.w #$2108                ; $858614
    sta.w $4320                 ; $858617
    lda.w #$8D35                ; $85861A
    sta.w $4322                 ; $85861D
    stz.w $4325                 ; $858620
    lda.w #$2200                ; $858623
    sta.w $4330                 ; $858626
    lda.w $00CC                 ; $858629
    sta.w $4332                 ; $85862C
    stz.w $4335                 ; $85862F
    lda.w #$0500                ; $858632
    sta.w $4370                 ; $858635
    ldx.w #$047E                ; $858638
    lda.w $00B4                 ; $85863B
    bne CODE_858643             ; $85863E
    ldx.w #$0480                ; $858640
CODE_858643:
    stx.w $4372                 ; $858643
    stz.w $4375                 ; $858646
    sep.b #$20                  ; $858649
    lda.b #$83                  ; $85864B
    sta.w $4354                 ; $85864D
    sta.w $4364                 ; $858650
    sta.w $4357                 ; $858653
    sta.w $4367                 ; $858656
    lda.b #$7E                  ; $858659
    sta.w $4347                 ; $85865B
    sta.w $4314                 ; $85865E
    sta.w $4334                 ; $858661
    sta.w $4374                 ; $858664
    sta.w $4344                 ; $858667
    lda.b #$83                  ; $85866A
    sta.w $4324                 ; $85866C
    lda.b #$70                  ; $85866F
    sta.w $420C                 ; $858671
    rep.b #$20                  ; $858674
    jsl.l $8396D5               ; $858676
    sep.b #$20                  ; $85867A
    lda.w $01B0                 ; $85867C
    sta.w $2123                 ; $85867F
    lda.w $01B1                 ; $858682
    sta.w $2125                 ; $858685
    lda.w $01B4                 ; $858688
    sta.w $2132                 ; $85868B
    lda.w $01B5                 ; $85868E
    sta.w $2132                 ; $858691
    rep.b #$20                  ; $858694
    lda.w $01B2                 ; $858696
    sta.w $2131                 ; $858699
    rep.b #$20                  ; $85869C
    lda.w $0026                 ; $85869E
    bit.w #$0010                ; $8586A1
    bne CODE_8586DF             ; $8586A4
    sed                         ; $8586A6
    lda.w $0244                 ; $8586A7
    bit.w #$0008                ; $8586AA
    beq CODE_8586DE             ; $8586AD
    dec.b $60                   ; $8586AF
    bne CODE_8586CC             ; $8586B1
    lda.w #$0003                ; $8586B3
    sta.b $60                   ; $8586B6
    lda.b $5A                   ; $8586B8
    clc                         ; $8586BA
    adc.w #$0001                ; $8586BB
    sta.b $5A                   ; $8586BE
    sec                         ; $8586C0
    sbc.w #$6000                ; $8586C1
    bcc CODE_8586DE             ; $8586C4
    sta.b $5A                   ; $8586C6
    inc.b $5C                   ; $8586C8
    bra CODE_8586DE             ; $8586CA
CODE_8586CC:
    lda.b $5A                   ; $8586CC
    clc                         ; $8586CE
    adc.w #$0002                ; $8586CF
    sta.b $5A                   ; $8586D2
    sec                         ; $8586D4
    sbc.w #$6000                ; $8586D5
    bcc CODE_8586DE             ; $8586D8
    incbin !baserom:$286DA..$286DE
CODE_8586DE:
    cld                         ; $8586DE
CODE_8586DF:
    ldx.w $00BE                 ; $8586DF
    stx.w $00BC                 ; $8586E2
    lda.w $0486,x               ; $8586E5
    and.w #$00FF                ; $8586E8
    beq CODE_858754             ; $8586EB
    sec                         ; $8586ED
    sbc.w #$0004                ; $8586EE
    sta.b $2E                   ; $8586F1
    sep.b #$20                  ; $8586F3
    lda.b #$09                  ; $8586F5
    sta.w $2105                 ; $8586F7
    rep.b #$20                  ; $8586FA
    lda.b $2E                   ; $8586FC
    db $F0,$02 ; beq            ; $8586FE
    bpl CODE_858751             ; $858700
    incbin !baserom:$28702..$28751
CODE_858751:
    cmp.w #$00D8                ; $858751
CODE_858754:
    bpl CODE_858776             ; $858754
    lda.w #$87BA                ; $858756
    sta.w $0005                 ; $858759
    lda.b $2E                   ; $85875C
    clc                         ; $85875E
    adc.w #$0004                ; $85875F
    sta.w $4209                 ; $858762
    lda.w #$0065                ; $858765
    sta.w $4207                 ; $858768
    sep.b #$20                  ; $85876B
    lda.b #$09                  ; $85876D
    sta.w $2105                 ; $85876F
    rep.b #$20                  ; $858772
    bra CODE_85878B             ; $858774
CODE_858776:
    lda.w #$00FF                ; $858776
    sta.w $4209                 ; $858779
    lda.w #$0078                ; $85877C
    sta.w $4207                 ; $85877F
    sep.b #$20                  ; $858782
    lda.b #$29                  ; $858784
    sta.w $2105                 ; $858786
    rep.b #$20                  ; $858789
CODE_85878B:
    lda.b $24                   ; $85878B
    and.w #$0003                ; $85878D
    bne CODE_858795             ; $858790
    jsr.w $9147                 ; $858792
CODE_858795:
    bit.w $047C                 ; $858795
    bmi CODE_8587B7             ; $858798
    lda.l $7E4E35               ; $85879A
    and.w #$00F8                ; $85879E
    ora.w #$0006                ; $8587A1
    sta.w $0460                 ; $8587A4
    lda.l $7E4E2D               ; $8587A7
    sta.w $0461                 ; $8587AB
    sep.b #$20                  ; $8587AE
    lda.b #$20                  ; $8587B0
    tsb.w $045B                 ; $8587B2
    rep.b #$20                  ; $8587B5
CODE_8587B7:
    jmp.w $838A                 ; $8587B7
    rep.b #$10                  ; $8587BA
    sep.b #$20                  ; $8587BC
    phb                         ; $8587BE
    pha                         ; $8587BF
    phx                         ; $8587C0
    phk                         ; $8587C1
    plb                         ; $8587C2
    ldx.w $00BC                 ; $8587C3
    lda.b #$FF                  ; $8587C6
    sta.w $4207                 ; $8587C8
    lda.b #$01                  ; $8587CB
    sta.w $4325                 ; $8587CD
    sta.w $4375                 ; $8587D0
    lda.b #$08                  ; $8587D3
    sta.w $4315                 ; $8587D5
    sta.w $4335                 ; $8587D8
    lda.b #$21                  ; $8587DB
    sta.w $2121                 ; $8587DD
    lda.w $4211                 ; $8587E0
    lda.w $0596,x               ; $8587E3
    wai                         ; $8587E6
    sta.w $420B                 ; $8587E7
    inx                         ; $8587EA
    lda.w $4211                 ; $8587EB
    lda.w $0486,x               ; $8587EE
    beq CODE_858802             ; $8587F1
    sta.w $4209                 ; $8587F3
    lda.b #$65                  ; $8587F6
    sta.w $4207                 ; $8587F8
    stx.w $00BC                 ; $8587FB
    plx                         ; $8587FE
    pla                         ; $8587FF
    plb                         ; $858800
    rti                         ; $858801
CODE_858802:
    ldx.w #$00FF                ; $858802
    stx.w $4209                 ; $858805
    ldx.w #$0078                ; $858808
    stx.w $4207                 ; $85880B
    plx                         ; $85880E
    pla                         ; $85880F
    plb                         ; $858810
    rti                         ; $858811
    incbin !baserom:$28812..$28814
    phb                         ; $858814
    pea.w $8383                 ; $858815
    plb                         ; $858818
    plb                         ; $858819
    lda.w #$7E0F                ; $85881A
    sta.w $2116                 ; $85881D
    lda.w $1E94                 ; $858820
    sta.w $2118                 ; $858823
    lda.w #$7C62                ; $858826
    sta.w $2116                 ; $858829
    ldy.w #$0000                ; $85882C
    lda.b $3E                   ; $85882F
    and.w #$FFFE                ; $858831
    tax                         ; $858834
    beq CODE_858843             ; $858835
CODE_858837:
    lda.w $A422,y               ; $858837
    sta.w $2118                 ; $85883A
    iny                         ; $85883D
    iny                         ; $85883E
    dex                         ; $85883F
    dex                         ; $858840
    bne CODE_858837             ; $858841
CODE_858843:
    lda.b $3E                   ; $858843
    bit.w #$0001                ; $858845
    beq CODE_858852             ; $858848
    lda.w $A430,y               ; $85884A
    sta.w $2118                 ; $85884D
    iny                         ; $858850
    iny                         ; $858851
CODE_858852:
    lda.w $A43E                 ; $858852
    cpy.w #$000E                ; $858855
    beq CODE_858864             ; $858858
CODE_85885A:
    sta.w $2118                 ; $85885A
    iny                         ; $85885D
    iny                         ; $85885E
    cpy.w #$000E                ; $85885F
    bne CODE_85885A             ; $858862
CODE_858864:
    lda.w #$7F21                ; $858864
    sta.w $2116                 ; $858867
    lda.w $1E96                 ; $85886A
    sta.w $2118                 ; $85886D
    lda.b $42                   ; $858870
    sta.w $2118                 ; $858872
    lda.b $46                   ; $858875
    sta.w $2118                 ; $858877
    lda.b $4A                   ; $85887A
    sta.w $2118                 ; $85887C
    lda.w #$7F41                ; $85887F
    sta.w $2116                 ; $858882
    lda.w $1E98                 ; $858885
    sta.w $2118                 ; $858888
    lda.b $44                   ; $85888B
    sta.w $2118                 ; $85888D
    lda.b $48                   ; $858890
    sta.w $2118                 ; $858892
    lda.b $4C                   ; $858895
    sta.w $2118                 ; $858897
    lda.w #$7C4D                ; $85889A
    sta.w $2116                 ; $85889D
    lda.b $5C                   ; $8588A0
    and.w #$000F                ; $8588A2
    asl a                       ; $8588A5
    tax                         ; $8588A6
    lda.w $A498,x               ; $8588A7
    sta.w $2118                 ; $8588AA
    lda.w #$7C4F                ; $8588AD
    sta.w $2116                 ; $8588B0
    lda.b $5B                   ; $8588B3
    and.w #$00F0                ; $8588B5
    lsr a                       ; $8588B8
    lsr a                       ; $8588B9
    lsr a                       ; $8588BA
    tax                         ; $8588BB
    lda.w $A484,x               ; $8588BC
    sta.w $2118                 ; $8588BF
    lda.b $5B                   ; $8588C2
    and.w #$000F                ; $8588C4
    asl a                       ; $8588C7
    tax                         ; $8588C8
    lda.w $A498,x               ; $8588C9
    sta.w $2118                 ; $8588CC
    lda.w #$7C52                ; $8588CF
    sta.w $2116                 ; $8588D2
    lda.b $5A                   ; $8588D5
    and.w #$00F0                ; $8588D7
    lsr a                       ; $8588DA
    lsr a                       ; $8588DB
    lsr a                       ; $8588DC
    tax                         ; $8588DD
    lda.w $A484,x               ; $8588DE
    sta.w $2118                 ; $8588E1
    lda.b $5A                   ; $8588E4
    and.w #$000F                ; $8588E6
    asl a                       ; $8588E9
    tax                         ; $8588EA
    lda.w $A484,x               ; $8588EB
    sta.w $2118                 ; $8588EE
    lda.w #$7C22                ; $8588F1
    sta.w $2116                 ; $8588F4
    lda.b $64                   ; $8588F7
    sta.w $2118                 ; $8588F9
    lda.b $66                   ; $8588FC
    sta.w $2118                 ; $8588FE
    lda.w #$7C3A                ; $858901
    sta.w $2116                 ; $858904
    lda.w $0235                 ; $858907
    bpl CODE_85890F             ; $85890A
    lda.w #$0000                ; $85890C
CODE_85890F:
    inc a                       ; $85890F
    cmp.w $1F06                 ; $858910
    bmi CODE_858918             ; $858913
    lda.w $1F06                 ; $858915
CODE_858918:
    asl a                       ; $858918
    tax                         ; $858919
    lda.w $A484,x               ; $85891A
    sta.w $2118                 ; $85891D
    sep.b #$20                  ; $858920
    lda.b #$81                  ; $858922
    sta.w $2115                 ; $858924
    rep.b #$20                  ; $858927
    lda.w #$7EDE                ; $858929
    sta.w $2116                 ; $85892C
    lda.b $6C                   ; $85892F
    sta.w $2118                 ; $858931
    lda.b $6E                   ; $858934
    sta.w $2118                 ; $858936
    lda.b $70                   ; $858939
    sta.w $2118                 ; $85893B
    lda.b $72                   ; $85893E
    sta.w $2118                 ; $858940
    lda.b $74                   ; $858943
    sta.w $2118                 ; $858945
    sep.b #$20                  ; $858948
    lda.b #$80                  ; $85894A
    sta.w $2115                 ; $85894C
    rep.b #$20                  ; $85894F
    lda.w #$5AE0                ; $858951
    sta.w $2116                 ; $858954
    lda.b $80                   ; $858957
    sta.w $2118                 ; $858959
    lda.b $82                   ; $85895C
    sta.w $2118                 ; $85895E
    lda.b $84                   ; $858961
    sta.w $2118                 ; $858963
    lda.b $86                   ; $858966
    sta.w $2118                 ; $858968
    lda.b $88                   ; $85896B
    sta.w $2118                 ; $85896D
    lda.b $8A                   ; $858970
    sta.w $2118                 ; $858972
    lda.b $8C                   ; $858975
    sta.w $2118                 ; $858977
    lda.b $8E                   ; $85897A
    sta.w $2118                 ; $85897C
    lda.w #$7F3C                ; $85897F
    sta.w $2116                 ; $858982
    lda.w $00A4                 ; $858985
    sta.w $2118                 ; $858988
    lda.w $00A6                 ; $85898B
    sta.w $2118                 ; $85898E
    lda.w #$7F5C                ; $858991
    sta.w $2116                 ; $858994
    lda.w $00A8                 ; $858997
    sta.w $2118                 ; $85899A
    lda.w $00AA                 ; $85899D
    sta.w $2118                 ; $8589A0
    lda.w $025B                 ; $8589A3
    bpl CODE_8589D9             ; $8589A6
    incbin !baserom:$289A8..$289D9
CODE_8589D9:
    lda.w #$7F3B                ; $8589D9
    sta.w $2116                 ; $8589DC
    lda.l $858ABB               ; $8589DF
    sta.w $2118                 ; $8589E3
    lda.w #$7F57                ; $8589E6
    sta.w $2116                 ; $8589E9
    lda.w #$000A                ; $8589EC
    sec                         ; $8589EF
    sbc.w $025C                 ; $8589F0
    beq CODE_858A0C             ; $8589F3
    tay                         ; $8589F5
    lda.l $858ABB               ; $8589F6
CODE_8589FA:
    dey                         ; $8589FA
    beq CODE_858A05             ; $8589FB
    sta.w $2118                 ; $8589FD
    dey                         ; $858A00
    bne CODE_8589FA             ; $858A01
    bra CODE_858A0C             ; $858A03
CODE_858A05:
    lda.l $858ABD               ; $858A05
    sta.w $2118                 ; $858A09
CODE_858A0C:
    ldy.w $025C                 ; $858A0C
    dey                         ; $858A0F
    dey                         ; $858A10
    bmi CODE_858A1E             ; $858A11
    lda.l $858ABF               ; $858A13
CODE_858A17:
    sta.w $2118                 ; $858A17
    dey                         ; $858A1A
    dey                         ; $858A1B
    bpl CODE_858A17             ; $858A1C
CODE_858A1E:
    lda.w #$7C76                ; $858A1E
    sta.w $2116                 ; $858A21
    ldx.w $014C                 ; $858A24
    inx                         ; $858A27
    ldy.w #$0157                ; $858A28
    tya                         ; $858A2B
    dex                         ; $858A2C
    bne CODE_858A32             ; $858A2D
    lda.w #$295E                ; $858A2F
CODE_858A32:
    sta.w $2118                 ; $858A32
    tya                         ; $858A35
    dex                         ; $858A36
    bne CODE_858A3C             ; $858A37
    incbin !baserom:$28A39..$28A3C
CODE_858A3C:
    sta.w $2118                 ; $858A3C
    tya                         ; $858A3F
    dex                         ; $858A40
    bne CODE_858A46             ; $858A41
    incbin !baserom:$28A43..$28A46
CODE_858A46:
    sta.w $2118                 ; $858A46
    tya                         ; $858A49
    dex                         ; $858A4A
    bne CODE_858A50             ; $858A4B
    incbin !baserom:$28A4D..$28A50
CODE_858A50:
    sta.w $2118                 ; $858A50
    tya                         ; $858A53
    dex                         ; $858A54
    bne CODE_858A5A             ; $858A55
    incbin !baserom:$28A57..$28A5A
CODE_858A5A:
    sta.w $2118                 ; $858A5A
    sep.b #$20                  ; $858A5D
    ldx.w #$4E61                ; $858A5F
    stx.w $4372                 ; $858A62
    lda.b #$7E                  ; $858A65
    sta.w $4374                 ; $858A67
    lda.l $7E4E4D               ; $858A6A
    clc                         ; $858A6E
    adc.l $7E4E4E               ; $858A6F
    sta.l $7E4E4E               ; $858A73
    bpl CODE_858A84             ; $858A77
    ldx.w #$A5A4                ; $858A79
    stx.w $4372                 ; $858A7C
    lda.b #$83                  ; $858A7F
    sta.w $4374                 ; $858A81
CODE_858A84:
    ldx.w #$7DE9                ; $858A84
    stx.w $2116                 ; $858A87
    ldx.w #$1801                ; $858A8A
    stx.w $4370                 ; $858A8D
    ldx.w #$001A                ; $858A90
    stx.w $4375                 ; $858A93
    lda.b #$80                  ; $858A96
    sta.w $420B                 ; $858A98
    ldx.w #$001A                ; $858A9B
    stx.w $4375                 ; $858A9E
    ldx.w #$7E09                ; $858AA1
    stx.w $2116                 ; $858AA4
    sta.w $420B                 ; $858AA7
    rep.b #$20                  ; $858AAA
    plb                         ; $858AAC
    rtl                         ; $858AAD
    incbin !baserom:$28AAE..$29147
    phb                         ; $859147
    pea.w $7E7E                 ; $859148
    plb                         ; $85914B
    plb                         ; $85914C
    ldx.w $2000                 ; $85914D
    lda.w $2008                 ; $859150
    sta.w $2000                 ; $859153
    sta.w $2002                 ; $859156
    sta.w $2004                 ; $859159
    sta.w $2006                 ; $85915C
    lda.w $2010                 ; $85915F
    sta.w $2008                 ; $859162
    sta.w $200A                 ; $859165
    sta.w $200C                 ; $859168
    sta.w $200E                 ; $85916B
    lda.w $2018                 ; $85916E
    sta.w $2010                 ; $859171
    sta.w $2012                 ; $859174
    sta.w $2014                 ; $859177
    sta.w $2016                 ; $85917A
    lda.w $2020                 ; $85917D
    sta.w $2018                 ; $859180
    sta.w $201A                 ; $859183
    sta.w $201C                 ; $859186
    sta.w $201E                 ; $859189
    lda.w $2028                 ; $85918C
    sta.w $2020                 ; $85918F
    sta.w $2022                 ; $859192
    sta.w $2024                 ; $859195
    sta.w $2026                 ; $859198
    lda.w $2030                 ; $85919B
    sta.w $2028                 ; $85919E
    sta.w $202A                 ; $8591A1
    sta.w $202C                 ; $8591A4
    sta.w $202E                 ; $8591A7
    lda.w $2038                 ; $8591AA
    sta.w $2030                 ; $8591AD
    sta.w $2032                 ; $8591B0
    sta.w $2034                 ; $8591B3
    sta.w $2036                 ; $8591B6
    stx.w $2038                 ; $8591B9
    stx.w $203A                 ; $8591BC
    stx.w $203C                 ; $8591BF
    stx.w $203E                 ; $8591C2
    ldx.w $2040                 ; $8591C5
    lda.w $2048                 ; $8591C8
    sta.w $2040                 ; $8591CB
    sta.w $2042                 ; $8591CE
    sta.w $2044                 ; $8591D1
    sta.w $2046                 ; $8591D4
    lda.w $2050                 ; $8591D7
    sta.w $2048                 ; $8591DA
    sta.w $204A                 ; $8591DD
    sta.w $204C                 ; $8591E0
    sta.w $204E                 ; $8591E3
    lda.w $2058                 ; $8591E6
    sta.w $2050                 ; $8591E9
    sta.w $2052                 ; $8591EC
    sta.w $2054                 ; $8591EF
    sta.w $2056                 ; $8591F2
    lda.w $2060                 ; $8591F5
    sta.w $2058                 ; $8591F8
    sta.w $205A                 ; $8591FB
    sta.w $205C                 ; $8591FE
    sta.w $205E                 ; $859201
    lda.w $2068                 ; $859204
    sta.w $2060                 ; $859207
    sta.w $2062                 ; $85920A
    sta.w $2064                 ; $85920D
    sta.w $2066                 ; $859210
    lda.w $2070                 ; $859213
    sta.w $2068                 ; $859216
    sta.w $206A                 ; $859219
    sta.w $206C                 ; $85921C
    sta.w $206E                 ; $85921F
    lda.w $2078                 ; $859222
    sta.w $2070                 ; $859225
    sta.w $2072                 ; $859228
    sta.w $2074                 ; $85922B
    sta.w $2076                 ; $85922E
    stx.w $2078                 ; $859231
    stx.w $207A                 ; $859234
    stx.w $207C                 ; $859237
    stx.w $207E                 ; $85923A
    ldx.w $2080                 ; $85923D
    lda.w $2088                 ; $859240
    sta.w $2080                 ; $859243
    sta.w $2082                 ; $859246
    sta.w $2084                 ; $859249
    sta.w $2086                 ; $85924C
    lda.w $2090                 ; $85924F
    sta.w $2088                 ; $859252
    sta.w $208A                 ; $859255
    sta.w $208C                 ; $859258
    sta.w $208E                 ; $85925B
    lda.w $2098                 ; $85925E
    sta.w $2090                 ; $859261
    sta.w $2092                 ; $859264
    sta.w $2094                 ; $859267
    sta.w $2096                 ; $85926A
    lda.w $20A0                 ; $85926D
    sta.w $2098                 ; $859270
    sta.w $209A                 ; $859273
    sta.w $209C                 ; $859276
    sta.w $209E                 ; $859279
    lda.w $20A8                 ; $85927C
    sta.w $20A0                 ; $85927F
    sta.w $20A2                 ; $859282
    sta.w $20A4                 ; $859285
    sta.w $20A6                 ; $859288
    lda.w $20B0                 ; $85928B
    sta.w $20A8                 ; $85928E
    sta.w $20AA                 ; $859291
    sta.w $20AC                 ; $859294
    sta.w $20AE                 ; $859297
    lda.w $20B8                 ; $85929A
    sta.w $20B0                 ; $85929D
    sta.w $20B2                 ; $8592A0
    sta.w $20B4                 ; $8592A3
    sta.w $20B6                 ; $8592A6
    stx.w $20B8                 ; $8592A9
    stx.w $20BA                 ; $8592AC
    stx.w $20BC                 ; $8592AF
    stx.w $20BE                 ; $8592B2
    ldx.w $20C0                 ; $8592B5
    lda.w $20C8                 ; $8592B8
    sta.w $20C0                 ; $8592BB
    sta.w $20C2                 ; $8592BE
    sta.w $20C4                 ; $8592C1
    sta.w $20C6                 ; $8592C4
    lda.w $20D0                 ; $8592C7
    sta.w $20C8                 ; $8592CA
    sta.w $20CA                 ; $8592CD
    sta.w $20CC                 ; $8592D0
    sta.w $20CE                 ; $8592D3
    lda.w $20D8                 ; $8592D6
    sta.w $20D0                 ; $8592D9
    sta.w $20D2                 ; $8592DC
    sta.w $20D4                 ; $8592DF
    sta.w $20D6                 ; $8592E2
    lda.w $20E0                 ; $8592E5
    sta.w $20D8                 ; $8592E8
    sta.w $20DA                 ; $8592EB
    sta.w $20DC                 ; $8592EE
    sta.w $20DE                 ; $8592F1
    lda.w $20E8                 ; $8592F4
    sta.w $20E0                 ; $8592F7
    sta.w $20E2                 ; $8592FA
    sta.w $20E4                 ; $8592FD
    sta.w $20E6                 ; $859300
    lda.w $20F0                 ; $859303
    sta.w $20E8                 ; $859306
    sta.w $20EA                 ; $859309
    sta.w $20EC                 ; $85930C
    sta.w $20EE                 ; $85930F
    lda.w $20F8                 ; $859312
    sta.w $20F0                 ; $859315
    sta.w $20F2                 ; $859318
    sta.w $20F4                 ; $85931B
    sta.w $20F6                 ; $85931E
    stx.w $20F8                 ; $859321
    stx.w $20FA                 ; $859324
    stx.w $20FC                 ; $859327
    stx.w $20FE                 ; $85932A
    ldx.w $2100                 ; $85932D
    lda.w $2108                 ; $859330
    sta.w $2100                 ; $859333
    sta.w $2102                 ; $859336
    sta.w $2104                 ; $859339
    sta.w $2106                 ; $85933C
    lda.w $2110                 ; $85933F
    sta.w $2108                 ; $859342
    sta.w $210A                 ; $859345
    sta.w $210C                 ; $859348
    sta.w $210E                 ; $85934B
    lda.w $2118                 ; $85934E
    sta.w $2110                 ; $859351
    sta.w $2112                 ; $859354
    sta.w $2114                 ; $859357
    sta.w $2116                 ; $85935A
    lda.w $2120                 ; $85935D
    sta.w $2118                 ; $859360
    sta.w $211A                 ; $859363
    sta.w $211C                 ; $859366
    sta.w $211E                 ; $859369
    lda.w $2128                 ; $85936C
    sta.w $2120                 ; $85936F
    sta.w $2122                 ; $859372
    sta.w $2124                 ; $859375
    sta.w $2126                 ; $859378
    lda.w $2130                 ; $85937B
    sta.w $2128                 ; $85937E
    sta.w $212A                 ; $859381
    sta.w $212C                 ; $859384
    sta.w $212E                 ; $859387
    lda.w $2138                 ; $85938A
    sta.w $2130                 ; $85938D
    sta.w $2132                 ; $859390
    sta.w $2134                 ; $859393
    sta.w $2136                 ; $859396
    stx.w $2138                 ; $859399
    stx.w $213A                 ; $85939C
    stx.w $213C                 ; $85939F
    stx.w $213E                 ; $8593A2
    plb                         ; $8593A5
    rts                         ; $8593A6
    incbin !baserom:$293A7..$295BD
    rep.b #$20                  ; $8595BD
    phb                         ; $8595BF
    phk                         ; $8595C0
    plb                         ; $8595C1
    bit.w $4210                 ; $8595C2
    plb                         ; $8595C5
    rti                         ; $8595C6
    incbin !baserom:$295C7..$29ADB
    phb                         ; $859ADB
    phk                         ; $859ADC
    plb                         ; $859ADD
    lda.w #$00B9                ; $859ADE
    sta.b $18                   ; $859AE1
    lda.w #$0000                ; $859AE3
    sta.b $1A                   ; $859AE6
    stz.b $1C                   ; $859AE8
    lda.w #$0040                ; $859AEA
    sta.b $16                   ; $859AED
    lda.w $019A                 ; $859AEF
    sec                         ; $859AF2
    sbc.w #$0014                ; $859AF3
    sta.b $14                   ; $859AF6
    lda.l $8103DD               ; $859AF8
    and.w #$0003                ; $859AFC
    tax                         ; $859AFF
    lda.l $859FA7,x             ; $859B00
    and.w #$00FF                ; $859B04
    sta.b $1E                   ; $859B07
    lda.w $047B                 ; $859B09
    asl a                       ; $859B0C
    tax                         ; $859B0D
    lda.w $03DD                 ; $859B0E
    and.w #$0003                ; $859B11
    db $F0,$0F ; beq            ; $859B14
    dec a                       ; $859B16
    beq CODE_859B1F             ; $859B17
    lda.l $859E2F,x             ; $859B19
    bra CODE_859B29             ; $859B1D
CODE_859B1F:
    lda.l $859E8D,x             ; $859B1F
    bra CODE_859B29             ; $859B23
    incbin !baserom:$29B25..$29B29
CODE_859B29:
    clc                         ; $859B29
    adc.b $1E                   ; $859B2A
    sta.b $1E                   ; $859B2C
    lda.l $859F49,x             ; $859B2E
    sta.b $20                   ; $859B32
    lda.w $0391                 ; $859B34
    and.w #$FF00                ; $859B37
    asl a                       ; $859B3A
    sta.b $22                   ; $859B3B
    lda.w $03C0                 ; $859B3D
    and.w #$FF00                ; $859B40
    asl a                       ; $859B43
    sta.b $24                   ; $859B44
    ldx.w #$0000                ; $859B46
    ldy.w #$0000                ; $859B49
    lda.b $1C                   ; $859B4C
    sta.l $7EFADD,x             ; $859B4E
    inc.b $1C                   ; $859B52
    lda.w #$0000                ; $859B54
    sta.l $7EFAB5,x             ; $859B57
    sta.l $7EFAB7,x             ; $859B5B
    sta.l $7EFABD,x             ; $859B5F
    sta.l $7EFABF,x             ; $859B63
    sta.l $7EFAB9,x             ; $859B67
    sta.l $7EFABB,x             ; $859B6B
    sta.l $7EFAC5,x             ; $859B6F
    sta.l $7EFADE,x             ; $859B73
    lda.w #$FFFF                ; $859B77
    sta.l $7EFAD3,x             ; $859B7A
    lda.w $019C                 ; $859B7E
    sta.l $7EFAE0,x             ; $859B81
    lda.w #$7000                ; $859B85
    sta.l $7EFAE2,x             ; $859B88
    lda.b $18                   ; $859B8C
    lda.w #$0000                ; $859B8E
    sta.l $7EFACB,x             ; $859B91
    sec                         ; $859B95
    sbc.w #$0005                ; $859B96
    bpl CODE_859B9E             ; $859B99
    lda.w #$0000                ; $859B9B
CODE_859B9E:
    sta.b $18                   ; $859B9E
    lda.b $1A                   ; $859BA0
    sta.l $7EFAD1,x             ; $859BA2
CODE_859BA6:
    clc                         ; $859BA6
    adc.w #$0200                ; $859BA7
    cmp.w #$0E00                ; $859BAA
    bne CODE_859BB2             ; $859BAD
    lda.w #$0000                ; $859BAF
CODE_859BB2:
    cmp.b $22                   ; $859BB2
    beq CODE_859BA6             ; $859BB4
    cmp.b $24                   ; $859BB6
    beq CODE_859BA6             ; $859BB8
    sta.b $1A                   ; $859BBA
    jsl.l rng_next_long         ; $859BBC
    and.w #$001F                ; $859BC0
    eor.w #$FFFF                ; $859BC3
    sec                         ; $859BC6
    adc.w $9DDF,y               ; $859BC7
    sec                         ; $859BCA
    sbc.b $1E                   ; $859BCB
    sta.l $7EFAD7,x             ; $859BCD
    lda.w $9E07,y               ; $859BD1
    sec                         ; $859BD4
    sbc.b $20                   ; $859BD5
    sta.l $7EFAD9,x             ; $859BD7
CODE_859BDB:
    jsl.l rng_next_long         ; $859BDB
    cmp.w #$FFF0                ; $859BDF
    bmi CODE_859BE9             ; $859BE2
    cmp.w #$0010                ; $859BE4
    bmi CODE_859BDB             ; $859BE7
CODE_859BE9:
    and.w #$81FF                ; $859BE9
    bpl CODE_859BF1             ; $859BEC
    ora.w #$FE00                ; $859BEE
CODE_859BF1:
    sta.l $7EFADB,x             ; $859BF1
    txa                         ; $859BF5
    clc                         ; $859BF6
    adc.w #$0035                ; $859BF7
    tax                         ; $859BFA
    iny                         ; $859BFB
    iny                         ; $859BFC
    cpy.w #$0028                ; $859BFD
    beq CODE_859C05             ; $859C00
    jmp.w $9B4C                 ; $859C02
CODE_859C05:
    lda.w $0391                 ; $859C05
    and.w #$FF00                ; $859C08
    asl a                       ; $859C0B
    sta.l $7EFAD1               ; $859C0C
    lda.w $03C0                 ; $859C10
    and.w #$FF00                ; $859C13
    asl a                       ; $859C16
    sta.l $7EFB06               ; $859C17
    lda.w #$0040                ; $859C1B
    sta.b $16                   ; $859C1E
    lda.w $019A                 ; $859C20
    sec                         ; $859C23
    sbc.w #$0014                ; $859C24
    sta.b $14                   ; $859C27
    ldy.w #$0000                ; $859C29
CODE_859C2C:
    ldx.w $1E6A,y               ; $859C2C
    lda.b $14                   ; $859C2F
    sta.l $7EFAB6,x             ; $859C31
    clc                         ; $859C35
    adc.w #$0001                ; $859C36
    sta.b $14                   ; $859C39
    lda.b $16                   ; $859C3B
    sta.l $7EFABB,x             ; $859C3D
    eor.w #$FF80                ; $859C41
    sta.b $16                   ; $859C44
    iny                         ; $859C46
    iny                         ; $859C47
    cpy.w #$0028                ; $859C48
    bne CODE_859C2C             ; $859C4B
    plb                         ; $859C4D
    ldy.w #$0000                ; $859C4E
    stz.b $14                   ; $859C51
    stz.b $16                   ; $859C53
CODE_859C55:
    ldx.w $1E6A,y               ; $859C55
    lda.b $14                   ; $859C58
    sta.l $7EFAE6,x             ; $859C5A
    clc                         ; $859C5E
    adc.w #$0600                ; $859C5F
    sta.b $14                   ; $859C62
    lda.b $16                   ; $859C64
    sta.l $7EFAE8,x             ; $859C66
    clc                         ; $859C6A
    adc.w #$0300                ; $859C6B
    sta.b $16                   ; $859C6E
    iny                         ; $859C70
    iny                         ; $859C71
    cpy.w #$0028                ; $859C72
    bne CODE_859C55             ; $859C75
    lda.w #$0002                ; $859C77
    sta.l $7EFADE               ; $859C7A
    sta.l $7EFB13               ; $859C7E
    lda.w #$FFFF                ; $859C82
    sta.l $7EFED9               ; $859C85
    lda.w #$7FFF                ; $859C89
    sta.l $7EFEDB               ; $859C8C
    lda.l $7EFAB5               ; $859C90
    sec                         ; $859C94
    sbc.w #$4040                ; $859C95
    sta.w $0225                 ; $859C98
    lda.l $7EFAB7               ; $859C9B
    sbc.w #$0000                ; $859C9F
    sta.w $0227                 ; $859CA2
    lda.w #$0002                ; $859CA5
    sta.l $7EFAC5               ; $859CA8
    ldx.w #$0035                ; $859CAC
    lda.l $7EFAEA               ; $859CAF
    sec                         ; $859CB3
    sbc.w #$4040                ; $859CB4
    sta.w $02D8                 ; $859CB7
    lda.l $7EFAEC               ; $859CBA
    sbc.w #$0000                ; $859CBE
    sta.w $02DA                 ; $859CC1
    lda.w #$0004                ; $859CC4
    sta.l $7EFAFA               ; $859CC7
    lda.w $0226                 ; $859CCB
    sec                         ; $859CCE
    sbc.w #$0020                ; $859CCF
    sta.w $023E                 ; $859CD2
    lda.w $02D9                 ; $859CD5
    sec                         ; $859CD8
    sbc.w #$0020                ; $859CD9
    sta.w $02F1                 ; $859CDC
    lda.w #$FFFF                ; $859CDF
    sta.l $7EFED9               ; $859CE2
    lda.w #$7FFF                ; $859CE6
    sta.l $7EFEDB               ; $859CE9
    lda.w #$0424                ; $859CED
    sta.w $1E92                 ; $859CF0
    lda.w #$FFFF                ; $859CF3
    sta.w $1EDE                 ; $859CF6
    sta.w $1EE2                 ; $859CF9
    jsr.w $9D39                 ; $859CFC
    jsr.w $9D5A                 ; $859CFF
    jsl.l $82A9AF               ; $859D02
    jsl.l $82A9B3               ; $859D06
    stz.w $1EE6                 ; $859D0A
    stz.w $1EE8                 ; $859D0D
    lda.w #$FFFF                ; $859D10
    sta.w $1EDE                 ; $859D13
    sta.w $1EE2                 ; $859D16
    lda.l $7EFAB9               ; $859D19
    sta.w $0204                 ; $859D1D
    lda.l $7EFABB               ; $859D20
    sta.w $0206                 ; $859D24
    ldx.w #$0035                ; $859D27
    lda.l $7EFAB9,x             ; $859D2A
    sta.w $02B7                 ; $859D2E
    lda.l $7EFABB,x             ; $859D31
    sta.w $02B9                 ; $859D35
    rtl                         ; $859D38
    lda.b $9C                   ; $859D39
    clc                         ; $859D3B
    adc.b $A2                   ; $859D3C
    tax                         ; $859D3E
    lda.w $023E                 ; $859D3F
    sec                         ; $859D42
    sbc.l $840000,x             ; $859D43
    bcc CODE_859D59             ; $859D47
    cmp.w #$0040                ; $859D49
    bcc CODE_859D59             ; $859D4C
    lda.b $A2                   ; $859D4E
    clc                         ; $859D50
    adc.w #$0014                ; $859D51
    sta.b $A2                   ; $859D54
    jmp.w $9D39                 ; $859D56
CODE_859D59:
    rts                         ; $859D59
    lda.b $9C                   ; $859D5A
    clc                         ; $859D5C
    adc.b $A6                   ; $859D5D
    tax                         ; $859D5F
    lda.w $02F1                 ; $859D60
    sec                         ; $859D63
    sbc.l $840000,x             ; $859D64
    bcc CODE_859D7A             ; $859D68
    cmp.w #$0040                ; $859D6A
    bcc CODE_859D7A             ; $859D6D
    lda.b $A6                   ; $859D6F
    clc                         ; $859D71
    adc.w #$0014                ; $859D72
    sta.b $A6                   ; $859D75
    jmp.w $9D5A                 ; $859D77
CODE_859D7A:
    rts                         ; $859D7A
    ldx.w $047B                 ; $859D7B
    lda.l $85A439,x             ; $859D7E
    bpl CODE_859D9F             ; $859D82
    incbin !baserom:$29D84..$29D9F
CODE_859D9F:
    ldx.w #$95E3                ; $859D9F
    ldy.w #$6C33                ; $859DA2
    lda.w #$7E8C                ; $859DA5
    jsl.l $82FB70               ; $859DA8
    rtl                         ; $859DAC
    incbin !baserom:$29DAD..$2A066
    pea.w $837E                 ; $85A066
    plb                         ; $85A069
    ldx.w #$0000                ; $85A06A
    txa                         ; $85A06D
    ldy.w #$0014                ; $85A06E
CODE_85A071:
    sta.w $4CFD,x               ; $85A071
    inc a                       ; $85A074
    inc a                       ; $85A075
    inx                         ; $85A076
    inx                         ; $85A077
    dey                         ; $85A078
    bne CODE_85A071             ; $85A079
    lda.w #$0026                ; $85A07B
    sta.b $14                   ; $85A07E
CODE_85A080:
    stz.b $16                   ; $85A080
CODE_85A082:
    ldx.b $16                   ; $85A082
    lda.w $4CFD,x               ; $85A084
    tay                         ; $85A087
    lda.w $4CFF,x               ; $85A088
    tax                         ; $85A08B
    lda.w $6991,y               ; $85A08C
    cmp.w $6991,x               ; $85A08F
    lda.w $69B9,y               ; $85A092
    sbc.w $69B9,x               ; $85A095
    lda.w $69E1,y               ; $85A098
    sbc.w $69E1,x               ; $85A09B
    bcs CODE_85A0B0             ; $85A09E
    ldx.b $16                   ; $85A0A0
    lda.w $4CFD,x               ; $85A0A2
    pha                         ; $85A0A5
    lda.w $4CFF,x               ; $85A0A6
    sta.w $4CFD,x               ; $85A0A9
    pla                         ; $85A0AC
    sta.w $4CFF,x               ; $85A0AD
CODE_85A0B0:
    ldx.b $16                   ; $85A0B0
    inx                         ; $85A0B2
    inx                         ; $85A0B3
    stx.b $16                   ; $85A0B4
    cpx.b $14                   ; $85A0B6
    bne CODE_85A082             ; $85A0B8
    dec.b $14                   ; $85A0BA
    dec.b $14                   ; $85A0BC
    bne CODE_85A080             ; $85A0BE
    ldy.w #$0000                ; $85A0C0
CODE_85A0C3:
    lda.w $4CFD,y               ; $85A0C3
    tax                         ; $85A0C6
    lda.w $6A09,x               ; $85A0C7
    and.w #$0004                ; $85A0CA
    bne CODE_85A0DF             ; $85A0CD
    sep.b #$20                  ; $85A0CF
    txa                         ; $85A0D1
    lsr a                       ; $85A0D2
    ldx.w $6A31                 ; $85A0D3
    sta.w $0446,x               ; $85A0D6
    inx                         ; $85A0D9
    stx.w $6A31                 ; $85A0DA
    rep.b #$20                  ; $85A0DD
CODE_85A0DF:
    iny                         ; $85A0DF
    iny                         ; $85A0E0
    cpy.w #$0028                ; $85A0E1
    bne CODE_85A0C3             ; $85A0E4
    plb                         ; $85A0E6
    lda.w #$FFFF                ; $85A0E7
    sta.w $03A8                 ; $85A0EA
    sta.w $03D7                 ; $85A0ED
    ldx.w #$0000                ; $85A0F0
    ldy.w #$000A                ; $85A0F3
CODE_85A0F6:
    lda.w $0446,x               ; $85A0F6
    and.w #$00FF                ; $85A0F9
    bne CODE_85A104             ; $85A0FC
    txa                         ; $85A0FE
    sta.w $03A8                 ; $85A0FF
    bra CODE_85A10B             ; $85A102
CODE_85A104:
    dec a                       ; $85A104
    bne CODE_85A10B             ; $85A105
    txa                         ; $85A107
    sta.w $03D7                 ; $85A108
CODE_85A10B:
    inx                         ; $85A10B
    lda.w $03A8                 ; $85A10C
    ora.w $03D7                 ; $85A10F
    bpl CODE_85A117             ; $85A112
    dey                         ; $85A114
    bne CODE_85A0F6             ; $85A115
CODE_85A117:
    rtl                         ; $85A117
    incbin !baserom:$2A118..$2A471
    pea.w $8330                 ; $85A471
    plb                         ; $85A474
    lda.w #$0000                ; $85A475
    sta.w $00E0                 ; $85A478
    lda.w #$06C2                ; $85A47B
    sta.w $00D0                 ; $85A47E
    lda.w $00B4                 ; $85A481
    beq CODE_85A492             ; $85A484
    incbin !baserom:$2A486..$2A492
CODE_85A492:
    ldy.w #$0023                ; $85A492
    ldx.w #$2B2B                ; $85A495
    lda.w #$019C                ; $85A498
    sta.b $1A                   ; $85A49B
    lda.w #$0004                ; $85A49D
    sta.b $1C                   ; $85A4A0
    lda.w $00B4                 ; $85A4A2
    bne CODE_85A4B7             ; $85A4A5
    ldy.w #$0045                ; $85A4A7
    ldx.w #$2C2B                ; $85A4AA
    lda.w #$033C                ; $85A4AD
    sta.b $1A                   ; $85A4B0
    lda.w #$01A4                ; $85A4B2
    sta.b $1C                   ; $85A4B5
CODE_85A4B7:
    stx.b $18                   ; $85A4B7
    lda.w $0287                 ; $85A4B9
    lsr a                       ; $85A4BC
    and.w #$007E                ; $85A4BD
    eor.w #$FFFF                ; $85A4C0
    sec                         ; $85A4C3
    adc.w #$0080                ; $85A4C4
    sta.b $14                   ; $85A4C7
    lda.w $0288                 ; $85A4C9
    asl a                       ; $85A4CC
    and.w #$003E                ; $85A4CD
    sta.b $16                   ; $85A4D0
    ldx.b $16                   ; $85A4D2
    stz.w $8000                 ; $85A4D4
    lda.l $7E3DEC,x             ; $85A4D7
    sta.w $8000                 ; $85A4DB
    lda.w #$FC00                ; $85A4DE
    sta.w $8000                 ; $85A4E1
    nop                         ; $85A4E4
    lda.w $8000                 ; $85A4E5
    clc                         ; $85A4E8
    adc.w #$8000                ; $85A4E9
    lda.w $8000                 ; $85A4EC
    adc.w #$0000                ; $85A4EF
    sta.w $0178                 ; $85A4F2
    stz.w $8000                 ; $85A4F5
    lda.l $7E3E6E,x             ; $85A4F8
    sta.w $8000                 ; $85A4FC
    lda.w #$0080                ; $85A4FF
    sec                         ; $85A502
    sbc.b $14                   ; $85A503
    lsr a                       ; $85A505
    xba                         ; $85A506
    sta.w $8000                 ; $85A507
    nop                         ; $85A50A
    lda.w $8000                 ; $85A50B
    sta.b $1E                   ; $85A50E
    lda.w $8000                 ; $85A510
    asl.b $1E                   ; $85A513
    rol a                       ; $85A515
    asl.b $1E                   ; $85A516
    rol a                       ; $85A518
    eor.w #$FFFF                ; $85A519
    inc a                       ; $85A51C
    sta.b $1E                   ; $85A51D
    lda.w $026A                 ; $85A51F
    clc                         ; $85A522
    adc.b $1E                   ; $85A523
    sta.b $1E                   ; $85A525
    stz.w $8000                 ; $85A527
    lda.b $1E                   ; $85A52A
    sta.w $8000                 ; $85A52C
    lda.b $14                   ; $85A52F
    lsr a                       ; $85A531
    xba                         ; $85A532
    sta.w $8000                 ; $85A533
    nop                         ; $85A536
    lda.w $8000                 ; $85A537
    sta.b $20                   ; $85A53A
    lda.w $8000                 ; $85A53C
    asl.b $20                   ; $85A53F
    rol a                       ; $85A541
    asl.b $20                   ; $85A542
    rol a                       ; $85A544
    stz.b $20                   ; $85A545
    stz.w $8000                 ; $85A547
    lda.l $7E3DEE,x             ; $85A54A
    sta.w $8000                 ; $85A54E
    lda.w #$0080                ; $85A551
    sec                         ; $85A554
    sbc.b $14                   ; $85A555
    lsr a                       ; $85A557
    xba                         ; $85A558
    sta.w $8000                 ; $85A559
    nop                         ; $85A55C
    lda.w $8000                 ; $85A55D
    sta.b $26                   ; $85A560
    lda.w $8000                 ; $85A562
    asl.b $26                   ; $85A565
    rol a                       ; $85A567
    asl.b $26                   ; $85A568
    rol a                       ; $85A56A
    eor.w #$FFFF                ; $85A56B
    inc a                       ; $85A56E
    sta.b $26                   ; $85A56F
    stz.w $8000                 ; $85A571
    lda.b $26                   ; $85A574
    sta.w $8000                 ; $85A576
    lda.b $14                   ; $85A579
    lsr a                       ; $85A57B
    xba                         ; $85A57C
    sta.w $8000                 ; $85A57D
    nop                         ; $85A580
    lda.w $8000                 ; $85A581
    sta.b $28                   ; $85A584
    lda.w $8000                 ; $85A586
    asl.b $28                   ; $85A589
    rol a                       ; $85A58B
    asl.b $28                   ; $85A58C
    rol a                       ; $85A58E
    sta.b $28                   ; $85A58F
    tyx                         ; $85A591
    lda.w #$0000                ; $85A592
    sta.l $7E4D95,x             ; $85A595
    lda.w #$000D                ; $85A599
    sta.w $8000                 ; $85A59C
    nop                         ; $85A59F
    nop                         ; $85A5A0
    lda.w #$0000                ; $85A5A1
    sta.w $8000                 ; $85A5A4
    lda.w #$0033                ; $85A5A7
    sta.w $8000                 ; $85A5AA
    clc                         ; $85A5AD
    adc.w #$00A9                ; $85A5AE
    sta.l $7E4D95,x             ; $85A5B1
    dex                         ; $85A5B5
    lda.w #$0033                ; $85A5B6
    sta.w $8000                 ; $85A5B9
    lda.w #$FFCB                ; $85A5BC
    sta.w $8000                 ; $85A5BF
    lda.w #$00BF                ; $85A5C2
    sta.w $8000                 ; $85A5C5
    lda.w #$00DC                ; $85A5C8
    sta.w $8000                 ; $85A5CB
    lda.w $0262                 ; $85A5CE
    sta.w $8000                 ; $85A5D1
    lda.w $0264                 ; $85A5D4
    sta.w $8000                 ; $85A5D7
    lda.w #$01C4                ; $85A5DA
    sta.w $8000                 ; $85A5DD
    lda.b $1A                   ; $85A5E0
    sta.w $8000                 ; $85A5E2
    lda.w #$00C0                ; $85A5E5
    sta.w $8000                 ; $85A5E8
    stx.b $22                   ; $85A5EB
    lda.w #$0002                ; $85A5ED
    sta.b $24                   ; $85A5F0
    lda.b $1D                   ; $85A5F2
    and.w #$FF00                ; $85A5F4
    sta.w $8000                 ; $85A5F7
    lda.b $1F                   ; $85A5FA
    and.w #$00FF                ; $85A5FC
    bit.w #$0080                ; $85A5FF
    beq CODE_85A607             ; $85A602
    incbin !baserom:$2A604..$2A607
CODE_85A607:
    sta.w $8000                 ; $85A607
    lda.b $25                   ; $85A60A
    and.w #$FF00                ; $85A60C
    sta.w $8000                 ; $85A60F
    lda.b $27                   ; $85A612
    and.w #$00FF                ; $85A614
    bit.w #$0080                ; $85A617
    beq CODE_85A61F             ; $85A61A
    incbin !baserom:$2A61C..$2A61F
CODE_85A61F:
    sta.w $8000                 ; $85A61F
    ldx.b $14                   ; $85A622
    lda.l $83ACEC,x             ; $85A624
    sta.w $8000                 ; $85A628
    sta.b $30                   ; $85A62B
    lda.b $20                   ; $85A62D
    sta.w $8000                 ; $85A62F
    lda.b $28                   ; $85A632
    sta.w $8000                 ; $85A634
    jmp.w $A643                 ; $85A637
    lda.l $83ACEC,x             ; $85A63A
    sta.w $8000                 ; $85A63E
    sta.b $30                   ; $85A641
    ldx.b $16                   ; $85A643
    lda.l $7E3E6E,x             ; $85A645
    sta.w $8000                 ; $85A649
    lda.l $7E3DEE,x             ; $85A64C
    sta.w $8000                 ; $85A650
    ldx.b $16                   ; $85A653
    lda.l $7E3EEE,x             ; $85A655
    sta.w $8000                 ; $85A659
    inx                         ; $85A65C
    inx                         ; $85A65D
    stx.b $16                   ; $85A65E
    ldx.b $24                   ; $85A660
    lda.w $8000                 ; $85A662
    sta.l $7E356C,x             ; $85A665
    lda.w $8000                 ; $85A669
    sta.l $7E366C,x             ; $85A66C
    lda.w $8000                 ; $85A670
    sta.l $7E376C,x             ; $85A673
    lda.w $8000                 ; $85A677
    sta.l $7E386C,x             ; $85A67A
    lda.b $30                   ; $85A67E
    sta.l $7E396C,x             ; $85A680
    lda.b $14                   ; $85A684
    sta.l $7E3A6C,x             ; $85A686
    inx                         ; $85A68A
    inx                         ; $85A68B
    stx.b $24                   ; $85A68C
CODE_85A68E:
    lda.w $C000                 ; $85A68E
    bpl CODE_85A68E             ; $85A691
    lda.w $8000                 ; $85A693
    beq CODE_85A6EE             ; $85A696
    incbin !baserom:$2A698..$2A6EE
CODE_85A6EE:
    lda.b $14                   ; $85A6EE
    clc                         ; $85A6F0
    adc.w #$0080                ; $85A6F1
    sta.b $14                   ; $85A6F4
    cmp.w #$0E00                ; $85A6F6
    bcs CODE_85A6FF             ; $85A6F9
    tax                         ; $85A6FB
    jmp.w $A63A                 ; $85A6FC
CODE_85A6FF:
    lda.w #$8000                ; $85A6FF
    sta.w $8000                 ; $85A702
    lda.b $24                   ; $85A705
    sta.b $94                   ; $85A707
    lda.w #$0000                ; $85A709
    sta.l $7E4DB8               ; $85A70C
    sta.l $7E4DDA               ; $85A710
    lda.b $22                   ; $85A714
    sta.w $0166                 ; $85A716
    ldy.b $18                   ; $85A719
    iny                         ; $85A71B
    sty.w $015E                 ; $85A71C
    ldy.w $00D0                 ; $85A71F
    lda.w #$0000                ; $85A722
    sta.w $0000,y               ; $85A725
    dey                         ; $85A728
    dey                         ; $85A729
    dey                         ; $85A72A
    ldx.b $22                   ; $85A72B
    lda.l $7E4D96,x             ; $85A72D
    and.w #$00FF                ; $85A731
    sec                         ; $85A734
    sbc.w #$0074                ; $85A735
    bpl CODE_85A73D             ; $85A738
    lda.w #$0000                ; $85A73A
CODE_85A73D:
    sta.b $14                   ; $85A73D
    eor.w #$FFFF                ; $85A73F
    sec                         ; $85A742
    adc.w #$006C                ; $85A743
    ora.w #$0080                ; $85A746
    sta.w $0000,y               ; $85A749
    lda.w #$0067                ; $85A74C
    sec                         ; $85A74F
    sbc.b $14                   ; $85A750
    asl a                       ; $85A752
    asl a                       ; $85A753
    eor.w #$FFFF                ; $85A754
    sec                         ; $85A757
    adc.b $1A                   ; $85A758
    clc                         ; $85A75A
    adc.w #$0AF6                ; $85A75B
    sta.w $0001,y               ; $85A75E
    dey                         ; $85A761
    dey                         ; $85A762
    dey                         ; $85A763
    lda.w $0299                 ; $85A764
    clc                         ; $85A767
    adc.w #$0040                ; $85A768
    bpl CODE_85A770             ; $85A76B
    incbin !baserom:$2A76D..$2A770
CODE_85A770:
    sta.b $16                   ; $85A770
    sec                         ; $85A772
    sbc.b $14                   ; $85A773
    bpl CODE_85A791             ; $85A775
    incbin !baserom:$2A777..$2A791
CODE_85A791:
    lda.w $0299                 ; $85A791
    bpl CODE_85A799             ; $85A794
    lda.w #$0000                ; $85A796
CODE_85A799:
    sta.b $16                   ; $85A799
    sec                         ; $85A79B
    sbc.b $14                   ; $85A79C
    bpl CODE_85A7B8             ; $85A79E
    incbin !baserom:$2A7A0..$2A7B8
CODE_85A7B8:
    lda.b $14                   ; $85A7B8
    beq CODE_85A7CB             ; $85A7BA
    incbin !baserom:$2A7BC..$2A7CB
CODE_85A7CB:
    sty.w $00D0                 ; $85A7CB
    ldx.w $00E0                 ; $85A7CE
    lda.w #$004B                ; $85A7D1
    sec                         ; $85A7D4
    sbc.w $0299                 ; $85A7D5
    sta.w $1548,x               ; $85A7D8
    ldy.w #$0023                ; $85A7DB
    ldx.w #$272B                ; $85A7DE
    lda.w #$019C                ; $85A7E1
    sta.b $1A                   ; $85A7E4
    lda.w #$0004                ; $85A7E6
    sta.b $1C                   ; $85A7E9
    lda.w $00B4                 ; $85A7EB
    bne CODE_85A800             ; $85A7EE
    ldy.w #$0045                ; $85A7F0
    ldx.w #$282B                ; $85A7F3
    lda.w #$033C                ; $85A7F6
    sta.b $1A                   ; $85A7F9
    lda.w #$01A4                ; $85A7FB
    sta.b $1C                   ; $85A7FE
CODE_85A800:
    stx.b $18                   ; $85A800
    lda.w $0225                 ; $85A802
    lsr a                       ; $85A805
    and.w #$007E                ; $85A806
    eor.w #$FFFF                ; $85A809
    sec                         ; $85A80C
    adc.w #$0080                ; $85A80D
    sta.b $14                   ; $85A810
    lda.w $0226                 ; $85A812
    asl a                       ; $85A815
    and.w #$003E                ; $85A816
    sta.b $16                   ; $85A819
    ldx.b $16                   ; $85A81B
    stz.w $8000                 ; $85A81D
    lda.l $7E3AEC,x             ; $85A820
    sta.w $8000                 ; $85A824
    lda.w #$FC00                ; $85A827
    sta.w $8000                 ; $85A82A
    nop                         ; $85A82D
    lda.w $8000                 ; $85A82E
    clc                         ; $85A831
    adc.w #$8000                ; $85A832
    lda.w $8000                 ; $85A835
    adc.w #$0000                ; $85A838
    sta.w $0174                 ; $85A83B
    stz.w $8000                 ; $85A83E
    lda.l $7E3B6E,x             ; $85A841
    sta.w $8000                 ; $85A845
    lda.w #$0080                ; $85A848
    sec                         ; $85A84B
    sbc.b $14                   ; $85A84C
    lsr a                       ; $85A84E
    xba                         ; $85A84F
    sta.w $8000                 ; $85A850
    nop                         ; $85A853
    lda.w $8000                 ; $85A854
    sta.b $1E                   ; $85A857
    lda.w $8000                 ; $85A859
    asl.b $1E                   ; $85A85C
    rol a                       ; $85A85E
    asl.b $1E                   ; $85A85F
    rol a                       ; $85A861
    eor.w #$FFFF                ; $85A862
    inc a                       ; $85A865
    sta.b $1E                   ; $85A866
    lda.w $0208                 ; $85A868
    clc                         ; $85A86B
    adc.b $1E                   ; $85A86C
    sta.b $1E                   ; $85A86E
    stz.w $8000                 ; $85A870
    lda.b $1E                   ; $85A873
    sta.w $8000                 ; $85A875
    lda.b $14                   ; $85A878
    lsr a                       ; $85A87A
    xba                         ; $85A87B
    sta.w $8000                 ; $85A87C
    nop                         ; $85A87F
    lda.w $8000                 ; $85A880
    sta.b $20                   ; $85A883
    lda.w $8000                 ; $85A885
    asl.b $20                   ; $85A888
    rol a                       ; $85A88A
    asl.b $20                   ; $85A88B
    rol a                       ; $85A88D
    stz.b $20                   ; $85A88E
    stz.w $8000                 ; $85A890
    lda.l $7E3AEE,x             ; $85A893
    sta.w $8000                 ; $85A897
    lda.w #$0080                ; $85A89A
    sec                         ; $85A89D
    sbc.b $14                   ; $85A89E
    lsr a                       ; $85A8A0
    xba                         ; $85A8A1
    sta.w $8000                 ; $85A8A2
    nop                         ; $85A8A5
    lda.w $8000                 ; $85A8A6
    sta.b $26                   ; $85A8A9
    lda.w $8000                 ; $85A8AB
    asl.b $26                   ; $85A8AE
    rol a                       ; $85A8B0
    asl.b $26                   ; $85A8B1
    rol a                       ; $85A8B3
    eor.w #$FFFF                ; $85A8B4
    inc a                       ; $85A8B7
    sta.b $26                   ; $85A8B8
    stz.w $8000                 ; $85A8BA
    lda.b $26                   ; $85A8BD
    sta.w $8000                 ; $85A8BF
    lda.b $14                   ; $85A8C2
    lsr a                       ; $85A8C4
    xba                         ; $85A8C5
    sta.w $8000                 ; $85A8C6
    nop                         ; $85A8C9
    lda.w $8000                 ; $85A8CA
    sta.b $28                   ; $85A8CD
    lda.w $8000                 ; $85A8CF
    asl.b $28                   ; $85A8D2
    rol a                       ; $85A8D4
    asl.b $28                   ; $85A8D5
    rol a                       ; $85A8D7
    sta.b $28                   ; $85A8D8
    tyx                         ; $85A8DA
    lda.w #$0000                ; $85A8DB
    sta.l $7E4CFD,x             ; $85A8DE
    lda.w #$000D                ; $85A8E2
    sta.w $8000                 ; $85A8E5
    nop                         ; $85A8E8
    lda.w #$0000                ; $85A8E9
    sta.w $8000                 ; $85A8EC
    lda.w #$0033                ; $85A8EF
    sta.w $8000                 ; $85A8F2
    clc                         ; $85A8F5
    adc.w #$0039                ; $85A8F6
    sta.l $7E4CFD,x             ; $85A8F9
    dex                         ; $85A8FD
    lda.w #$0033                ; $85A8FE
    sta.w $8000                 ; $85A901
    lda.w #$FFCB                ; $85A904
    sta.w $8000                 ; $85A907
    lda.w #$00BF                ; $85A90A
    sta.w $8000                 ; $85A90D
    lda.w #$006C                ; $85A910
    sta.w $8000                 ; $85A913
    lda.w $0200                 ; $85A916
    sta.w $8000                 ; $85A919
    lda.w $0202                 ; $85A91C
    sta.w $8000                 ; $85A91F
    lda.w #$01C4                ; $85A922
    sta.w $8000                 ; $85A925
    lda.b $1A                   ; $85A928
    sta.w $8000                 ; $85A92A
    lda.w #$00C0                ; $85A92D
    sta.w $8000                 ; $85A930
    stx.b $22                   ; $85A933
    lda.w #$0002                ; $85A935
    sta.b $24                   ; $85A938
    lda.b $1D                   ; $85A93A
    and.w #$FF00                ; $85A93C
    sta.w $8000                 ; $85A93F
    lda.b $1F                   ; $85A942
    and.w #$00FF                ; $85A944
    bit.w #$0080                ; $85A947
    beq CODE_85A94F             ; $85A94A
    ora.w #$FF00                ; $85A94C
CODE_85A94F:
    sta.w $8000                 ; $85A94F
    lda.b $25                   ; $85A952
    and.w #$FF00                ; $85A954
    sta.w $8000                 ; $85A957
    lda.b $27                   ; $85A95A
    and.w #$00FF                ; $85A95C
    bit.w #$0080                ; $85A95F
    beq CODE_85A967             ; $85A962
    incbin !baserom:$2A964..$2A967
CODE_85A967:
    sta.w $8000                 ; $85A967
    ldx.b $14                   ; $85A96A
    lda.l $83ACEC,x             ; $85A96C
    sta.w $8000                 ; $85A970
    sta.b $30                   ; $85A973
    lda.b $20                   ; $85A975
    sta.w $8000                 ; $85A977
    lda.b $28                   ; $85A97A
    sta.w $8000                 ; $85A97C
    jmp.w $A98B                 ; $85A97F
    lda.l $83ACEC,x             ; $85A982
    sta.w $8000                 ; $85A986
    sta.b $30                   ; $85A989
    ldx.b $16                   ; $85A98B
    lda.l $7E3B6E,x             ; $85A98D
    sta.w $8000                 ; $85A991
    lda.l $7E3AEE,x             ; $85A994
    sta.w $8000                 ; $85A998
    ldx.b $16                   ; $85A99B
    lda.l $7E3BEE,x             ; $85A99D
    sta.w $8000                 ; $85A9A1
    inx                         ; $85A9A4
    inx                         ; $85A9A5
    stx.b $16                   ; $85A9A6
    ldx.b $24                   ; $85A9A8
    lda.w $8000                 ; $85A9AA
    sta.l $7E34EC,x             ; $85A9AD
    lda.w $8000                 ; $85A9B1
    sta.l $7E35EC,x             ; $85A9B4
    lda.w $8000                 ; $85A9B8
    sta.l $7E36EC,x             ; $85A9BB
    lda.w $8000                 ; $85A9BF
    sta.l $7E37EC,x             ; $85A9C2
    lda.b $30                   ; $85A9C6
    sta.l $7E38EC,x             ; $85A9C8
    lda.b $14                   ; $85A9CC
    sta.l $7E39EC,x             ; $85A9CE
    inx                         ; $85A9D2
    inx                         ; $85A9D3
    stx.b $24                   ; $85A9D4
CODE_85A9D6:
    lda.w $C000                 ; $85A9D6
    bpl CODE_85A9D6             ; $85A9D9
    lda.w $8000                 ; $85A9DB
    beq CODE_85AA36             ; $85A9DE
    tay                         ; $85A9E0
    ldx.b $22                   ; $85A9E1
    sep.b #$20                  ; $85A9E3
    eor.b #$FF                  ; $85A9E5
    sec                         ; $85A9E7
    adc.l $7E4CFE,x             ; $85A9E8
    sta.l $7E4CFD,x             ; $85A9EC
    rep.b #$20                  ; $85A9F0
    phy                         ; $85A9F2
    ldx.b $16                   ; $85A9F3
    lda.l $7E3CEC,x             ; $85A9F5
    tax                         ; $85A9F9
    phb                         ; $85A9FA
    ldy.b $18                   ; $85A9FB
    lda.w #$0007                ; $85A9FD
    mvp $7E,$90                 ; $85AA00
    sty.b $18                   ; $85AA03
    plb                         ; $85AA05
    ply                         ; $85AA06
CODE_85AA07:
    lda.w $C000                 ; $85AA07
    bpl CODE_85AA07             ; $85AA0A
    dey                         ; $85AA0C
    beq CODE_85AA21             ; $85AA0D
CODE_85AA0F:
    ldx.w $8000                 ; $85AA0F
    lda.w $8000                 ; $85AA12
    sta.w $0798,x               ; $85AA15
    lda.w $8000                 ; $85AA18
    sta.w $0796,x               ; $85AA1B
    dey                         ; $85AA1E
    bne CODE_85AA0F             ; $85AA1F
CODE_85AA21:
    ldx.w $8000                 ; $85AA21
    lda.w $8000                 ; $85AA24
    sta.w $0798,x               ; $85AA27
    lda.w $8000                 ; $85AA2A
    sta.w $0796,x               ; $85AA2D
    dec.b $22                   ; $85AA30
    cpx.b $1C                   ; $85AA32
    bmi CODE_85AA47             ; $85AA34
CODE_85AA36:
    lda.b $14                   ; $85AA36
    clc                         ; $85AA38
    adc.w #$0080                ; $85AA39
    sta.b $14                   ; $85AA3C
    cmp.w #$0E00                ; $85AA3E
    bcs CODE_85AA47             ; $85AA41
    tax                         ; $85AA43
    jmp.w $A982                 ; $85AA44
CODE_85AA47:
    lda.w #$8000                ; $85AA47
    sta.w $8000                 ; $85AA4A
    lda.b $24                   ; $85AA4D
    sta.b $90                   ; $85AA4F
    lda.w #$0000                ; $85AA51
    sta.l $7E4D20               ; $85AA54
    sta.l $7E4D42               ; $85AA58
    lda.b $22                   ; $85AA5C
    sta.w $0164                 ; $85AA5E
    ldy.b $18                   ; $85AA61
    iny                         ; $85AA63
    sty.w $015C                 ; $85AA64
    ldy.w $00D0                 ; $85AA67
    lda.w #$0008                ; $85AA6A
    sta.w $0000,y               ; $85AA6D
    dey                         ; $85AA70
    dey                         ; $85AA71
    dey                         ; $85AA72
    ldx.b $22                   ; $85AA73
    lda.l $7E4CFE,x             ; $85AA75
    and.w #$00FF                ; $85AA79
    sec                         ; $85AA7C
    sbc.w #$0004                ; $85AA7D
    bpl CODE_85AA85             ; $85AA80
    incbin !baserom:$2AA82..$2AA85
CODE_85AA85:
    sta.b $14                   ; $85AA85
    eor.w #$FFFF                ; $85AA87
    sec                         ; $85AA8A
    adc.w #$0068                ; $85AA8B
    ora.w #$0080                ; $85AA8E
    sta.w $0000,y               ; $85AA91
    lda.w #$0067                ; $85AA94
    sec                         ; $85AA97
    sbc.b $14                   ; $85AA98
    asl a                       ; $85AA9A
    asl a                       ; $85AA9B
    eor.w #$FFFF                ; $85AA9C
    sec                         ; $85AA9F
    adc.b $1A                   ; $85AAA0
    clc                         ; $85AAA2
    adc.w #$0796                ; $85AAA3
    sta.w $0001,y               ; $85AAA6
    dey                         ; $85AAA9
    dey                         ; $85AAAA
    dey                         ; $85AAAB
    lda.w $0237                 ; $85AAAC
    clc                         ; $85AAAF
    adc.w #$0040                ; $85AAB0
    bpl CODE_85AAB8             ; $85AAB3
    incbin !baserom:$2AAB5..$2AAB8
CODE_85AAB8:
    sta.b $16                   ; $85AAB8
    sec                         ; $85AABA
    sbc.b $14                   ; $85AABB
    bpl CODE_85AAD9             ; $85AABD
    eor.w #$FFFF                ; $85AABF
    inc a                       ; $85AAC2
    ora.w #$0080                ; $85AAC3
    sta.w $0000,y               ; $85AAC6
    lda.b $16                   ; $85AAC9
    sta.b $14                   ; $85AACB
    asl a                       ; $85AACD
    asl a                       ; $85AACE
    clc                         ; $85AACF
    adc.w #$A968                ; $85AAD0
    sta.w $0001,y               ; $85AAD3
    dey                         ; $85AAD6
    dey                         ; $85AAD7
    dey                         ; $85AAD8
CODE_85AAD9:
    lda.w $0237                 ; $85AAD9
    bpl CODE_85AAE1             ; $85AADC
    lda.w #$0000                ; $85AADE
CODE_85AAE1:
    sta.b $16                   ; $85AAE1
    sec                         ; $85AAE3
    sbc.b $14                   ; $85AAE4
    bpl CODE_85AB00             ; $85AAE6
    eor.w #$FFFF                ; $85AAE8
    inc a                       ; $85AAEB
    sta.w $0000,y               ; $85AAEC
    lda.w $00E0                 ; $85AAEF
    clc                         ; $85AAF2
    adc.w #$1536                ; $85AAF3
    sta.w $0001,y               ; $85AAF6
    dey                         ; $85AAF9
    dey                         ; $85AAFA
    dey                         ; $85AAFB
    lda.b $16                   ; $85AAFC
    sta.b $14                   ; $85AAFE
CODE_85AB00:
    lda.b $14                   ; $85AB00
    beq CODE_85AB13             ; $85AB02
    incbin !baserom:$2AB04..$2AB13
CODE_85AB13:
    lda.w #$0004                ; $85AB13
    sta.w $0000,y               ; $85AB16
    sty.w $00D0                 ; $85AB19
    ldx.w $00E0                 ; $85AB1C
    lda.w #$00BB                ; $85AB1F
    sec                         ; $85AB22
    sbc.w $0237                 ; $85AB23
    sta.w $1538,x               ; $85AB26
    lda.w #$0000                ; $85AB29
    sta.w $00E0                 ; $85AB2C
    lda.w #$073A                ; $85AB2F
    sta.w $00D2                 ; $85AB32
    lda.w $00B4                 ; $85AB35
    beq CODE_85AB46             ; $85AB38
    incbin !baserom:$2AB3A..$2AB46
CODE_85AB46:
    ldy.w #$0023                ; $85AB46
    ldx.w #$2D2B                ; $85AB49
    lda.w #$019C                ; $85AB4C
    sta.b $1A                   ; $85AB4F
    lda.w #$0004                ; $85AB51
    sta.b $1C                   ; $85AB54
    lda.w $00B4                 ; $85AB56
    bne CODE_85AB6B             ; $85AB59
    ldy.w #$0045                ; $85AB5B
    ldx.w #$2E2B                ; $85AB5E
    lda.w #$033C                ; $85AB61
    sta.b $1A                   ; $85AB64
    lda.w #$01A4                ; $85AB66
    sta.b $1C                   ; $85AB69
CODE_85AB6B:
    stx.b $18                   ; $85AB6B
    lda.w $033A                 ; $85AB6D
    lsr a                       ; $85AB70
    and.w #$007E                ; $85AB71
    eor.w #$FFFF                ; $85AB74
    sec                         ; $85AB77
    adc.w #$0080                ; $85AB78
    sta.b $14                   ; $85AB7B
    lda.w $033B                 ; $85AB7D
    asl a                       ; $85AB80
    and.w #$003E                ; $85AB81
    sta.b $16                   ; $85AB84
    ldx.b $16                   ; $85AB86
    stz.w $8000                 ; $85AB88
    lda.l $7E436C,x             ; $85AB8B
    sta.w $8000                 ; $85AB8F
    lda.w #$FC00                ; $85AB92
    sta.w $8000                 ; $85AB95
    nop                         ; $85AB98
    lda.w $8000                 ; $85AB99
    clc                         ; $85AB9C
    adc.w #$8000                ; $85AB9D
    lda.w $8000                 ; $85ABA0
    adc.w #$0000                ; $85ABA3
    sta.w $0180                 ; $85ABA6
    stz.w $8000                 ; $85ABA9
    lda.l $7E43EE,x             ; $85ABAC
    sta.w $8000                 ; $85ABB0
    lda.w #$0080                ; $85ABB3
    sec                         ; $85ABB6
    sbc.b $14                   ; $85ABB7
    lsr a                       ; $85ABB9
    xba                         ; $85ABBA
    sta.w $8000                 ; $85ABBB
    nop                         ; $85ABBE
    lda.w $8000                 ; $85ABBF
    sta.b $1E                   ; $85ABC2
    lda.w $8000                 ; $85ABC4
    asl.b $1E                   ; $85ABC7
    rol a                       ; $85ABC9
    asl.b $1E                   ; $85ABCA
    rol a                       ; $85ABCC
    eor.w #$FFFF                ; $85ABCD
    inc a                       ; $85ABD0
    sta.b $1E                   ; $85ABD1
    lda.w $031D                 ; $85ABD3
    clc                         ; $85ABD6
    adc.b $1E                   ; $85ABD7
    sta.b $1E                   ; $85ABD9
    stz.w $8000                 ; $85ABDB
    lda.b $1E                   ; $85ABDE
    sta.w $8000                 ; $85ABE0
    lda.b $14                   ; $85ABE3
    lsr a                       ; $85ABE5
    xba                         ; $85ABE6
    sta.w $8000                 ; $85ABE7
    nop                         ; $85ABEA
    lda.w $8000                 ; $85ABEB
    sta.b $20                   ; $85ABEE
    lda.w $8000                 ; $85ABF0
    asl.b $20                   ; $85ABF3
    rol a                       ; $85ABF5
    asl.b $20                   ; $85ABF6
    rol a                       ; $85ABF8
    stz.b $20                   ; $85ABF9
    stz.w $8000                 ; $85ABFB
    lda.l $7E436E,x             ; $85ABFE
    sta.w $8000                 ; $85AC02
    lda.w #$0080                ; $85AC05
    sec                         ; $85AC08
    sbc.b $14                   ; $85AC09
    lsr a                       ; $85AC0B
    xba                         ; $85AC0C
    sta.w $8000                 ; $85AC0D
    nop                         ; $85AC10
    lda.w $8000                 ; $85AC11
    sta.b $26                   ; $85AC14
    lda.w $8000                 ; $85AC16
    asl.b $26                   ; $85AC19
    rol a                       ; $85AC1B
    asl.b $26                   ; $85AC1C
    rol a                       ; $85AC1E
    eor.w #$FFFF                ; $85AC1F
    inc a                       ; $85AC22
    sta.b $26                   ; $85AC23
    stz.w $8000                 ; $85AC25
    lda.b $26                   ; $85AC28
    sta.w $8000                 ; $85AC2A
    lda.b $14                   ; $85AC2D
    lsr a                       ; $85AC2F
    xba                         ; $85AC30
    sta.w $8000                 ; $85AC31
    nop                         ; $85AC34
    lda.w $8000                 ; $85AC35
    sta.b $28                   ; $85AC38
    lda.w $8000                 ; $85AC3A
    asl.b $28                   ; $85AC3D
    rol a                       ; $85AC3F
    asl.b $28                   ; $85AC40
    rol a                       ; $85AC42
    sta.b $28                   ; $85AC43
    tyx                         ; $85AC45
    lda.w #$0000                ; $85AC46
    sta.l $7E4DE1,x             ; $85AC49
    lda.w #$000D                ; $85AC4D
    sta.w $8000                 ; $85AC50
    nop                         ; $85AC53
    lda.w #$0000                ; $85AC54
    sta.w $8000                 ; $85AC57
    lda.w #$0033                ; $85AC5A
    sta.w $8000                 ; $85AC5D
    clc                         ; $85AC60
    adc.w #$00A9                ; $85AC61
    sta.l $7E4DE1,x             ; $85AC64
    dex                         ; $85AC68
    lda.w #$0033                ; $85AC69
    sta.w $8000                 ; $85AC6C
    lda.w #$FFCB                ; $85AC6F
    sta.w $8000                 ; $85AC72
    lda.w #$00BF                ; $85AC75
    sta.w $8000                 ; $85AC78
    lda.w #$00DC                ; $85AC7B
    sta.w $8000                 ; $85AC7E
    lda.w $0315                 ; $85AC81
    sta.w $8000                 ; $85AC84
    lda.w $0317                 ; $85AC87
    sta.w $8000                 ; $85AC8A
    lda.w #$013C                ; $85AC8D
    sta.w $8000                 ; $85AC90
    lda.b $1A                   ; $85AC93
    sta.w $8000                 ; $85AC95
    lda.w #$00C0                ; $85AC98
    sta.w $8000                 ; $85AC9B
    stx.b $22                   ; $85AC9E
    lda.w #$0002                ; $85ACA0
    sta.b $24                   ; $85ACA3
    lda.b $1D                   ; $85ACA5
    and.w #$FF00                ; $85ACA7
    sta.w $8000                 ; $85ACAA
    lda.b $1F                   ; $85ACAD
    and.w #$00FF                ; $85ACAF
    bit.w #$0080                ; $85ACB2
    beq CODE_85ACBA             ; $85ACB5
    ora.w #$FF00                ; $85ACB7
CODE_85ACBA:
    sta.w $8000                 ; $85ACBA
    lda.b $25                   ; $85ACBD
    and.w #$FF00                ; $85ACBF
    sta.w $8000                 ; $85ACC2
    lda.b $27                   ; $85ACC5
    and.w #$00FF                ; $85ACC7
    bit.w #$0080                ; $85ACCA
    beq CODE_85ACD2             ; $85ACCD
    ora.w #$FF00                ; $85ACCF
CODE_85ACD2:
    sta.w $8000                 ; $85ACD2
    ldx.b $14                   ; $85ACD5
    lda.l $83ACEC,x             ; $85ACD7
    sta.w $8000                 ; $85ACDB
    sta.b $30                   ; $85ACDE
    lda.b $20                   ; $85ACE0
    sta.w $8000                 ; $85ACE2
    lda.b $28                   ; $85ACE5
    sta.w $8000                 ; $85ACE7
    jmp.w $ACF6                 ; $85ACEA
    lda.l $83ACEC,x             ; $85ACED
    sta.w $8000                 ; $85ACF1
    sta.b $30                   ; $85ACF4
    ldx.b $16                   ; $85ACF6
    lda.l $7E43EE,x             ; $85ACF8
    sta.w $8000                 ; $85ACFC
    lda.l $7E436E,x             ; $85ACFF
    sta.w $8000                 ; $85AD03
    ldx.b $16                   ; $85AD06
    lda.l $7E446E,x             ; $85AD08
    sta.w $8000                 ; $85AD0C
    inx                         ; $85AD0F
    inx                         ; $85AD10
    stx.b $16                   ; $85AD11
    ldx.b $24                   ; $85AD13
    lda.w $8000                 ; $85AD15
    sta.l $7E35AC,x             ; $85AD18
    lda.w $8000                 ; $85AD1C
    sta.l $7E36AC,x             ; $85AD1F
    lda.w $8000                 ; $85AD23
    sta.l $7E37AC,x             ; $85AD26
    lda.w $8000                 ; $85AD2A
    sta.l $7E38AC,x             ; $85AD2D
    lda.b $30                   ; $85AD31
    sta.l $7E39AC,x             ; $85AD33
    lda.b $14                   ; $85AD37
    sta.l $7E3AAC,x             ; $85AD39
    inx                         ; $85AD3D
    inx                         ; $85AD3E
    stx.b $24                   ; $85AD3F
CODE_85AD41:
    lda.w $C000                 ; $85AD41
    bpl CODE_85AD41             ; $85AD44
    lda.w $8000                 ; $85AD46
    beq CODE_85ADA1             ; $85AD49
    tay                         ; $85AD4B
    ldx.b $22                   ; $85AD4C
    sep.b #$20                  ; $85AD4E
    eor.b #$FF                  ; $85AD50
    sec                         ; $85AD52
    adc.l $7E4DE2,x             ; $85AD53
    sta.l $7E4DE1,x             ; $85AD57
    rep.b #$20                  ; $85AD5B
    phy                         ; $85AD5D
    ldx.b $16                   ; $85AD5E
    lda.l $7E456C,x             ; $85AD60
    tax                         ; $85AD64
    phb                         ; $85AD65
    ldy.b $18                   ; $85AD66
    lda.w #$0007                ; $85AD68
    mvp $7E,$90                 ; $85AD6B
    sty.b $18                   ; $85AD6E
    plb                         ; $85AD70
    ply                         ; $85AD71
CODE_85AD72:
    lda.w $C000                 ; $85AD72
    bpl CODE_85AD72             ; $85AD75
    dey                         ; $85AD77
    db $F0,$12 ; beq            ; $85AD78
CODE_85AD7A:
    ldx.w $8000                 ; $85AD7A
    lda.w $8000                 ; $85AD7D
    sta.w $11B8,x               ; $85AD80
    lda.w $8000                 ; $85AD83
    sta.w $11B6,x               ; $85AD86
    dey                         ; $85AD89
    bne CODE_85AD7A             ; $85AD8A
    incbin !baserom:$2AD8C..$2ADA1
CODE_85ADA1:
    lda.b $14                   ; $85ADA1
    clc                         ; $85ADA3
    adc.w #$0080                ; $85ADA4
    sta.b $14                   ; $85ADA7
    cmp.w #$0E00                ; $85ADA9
    db $B0,$04 ; bcs            ; $85ADAC
    tax                         ; $85ADAE
    jmp.w $ACED                 ; $85ADAF
    incbin !baserom:$2ADB2..$2B1FF
    lda.w $0164                 ; $85B1FF
    sta.b $14                   ; $85B202
    lda.w $0168                 ; $85B204
    sta.b $16                   ; $85B207
    lda.w #$8080                ; $85B209
    sta.b $18                   ; $85B20C
    ldy.w #$0002                ; $85B20E
    tyx                         ; $85B211
    lda.w $00B4                 ; $85B212
    bne CODE_85B21D             ; $85B215
    ldy.w #$0034                ; $85B217
    ldx.w #$0000                ; $85B21A
CODE_85B21D:
    sty.w $00C0                 ; $85B21D
    stx.b $1A                   ; $85B220
    sep.b #$20                  ; $85B222
    ldx.b $14                   ; $85B224
    lda.l $7E4CFE,x             ; $85B226
    ldx.b $16                   ; $85B22A
    cmp.l $7E4D4A,x             ; $85B22C
    beq CODE_85B250             ; $85B230
    incbin !baserom:$2B232..$2B250
CODE_85B250:
    ldx.b $1A                   ; $85B250
    lda.b #$39                  ; $85B252
    sta.w $047E,x               ; $85B254
CODE_85B257:
    ldx.b $14                   ; $85B257
    lda.l $7E4CFE,x             ; $85B259
    ldx.b $16                   ; $85B25D
    cmp.l $7E4D4A,x             ; $85B25F
    beq CODE_85B299             ; $85B263
    incbin !baserom:$2B265..$2B299
CODE_85B299:
    cmp.b #$00                  ; $85B299
    sta.w $0486,y               ; $85B29B
    beq CODE_85B2B4             ; $85B29E
    lda.b #$0E                  ; $85B2A0
    ora.b $18                   ; $85B2A2
    ora.b $19                   ; $85B2A4
    sta.w $0596,y               ; $85B2A6
    stz.b $18                   ; $85B2A9
    stz.b $19                   ; $85B2AB
    iny                         ; $85B2AD
    inc.b $14                   ; $85B2AE
    inc.b $16                   ; $85B2B0
    bra CODE_85B257             ; $85B2B2
CODE_85B2B4:
    rep.b #$20                  ; $85B2B4
    rts                         ; $85B2B6
    incbin !baserom:$2B2B7..$2B36F
    jsl.l $85B3D7               ; $85B36F
    jsl.l $85B828               ; $85B373
    lda.w $00D0                 ; $85B377
    sta.w $1E46                 ; $85B37A
    sta.w $1E4A                 ; $85B37D
    jsl.l $85BC20               ; $85B380
    lda.w $015C                 ; $85B384
    sta.w $016C                 ; $85B387
    lda.w $015E                 ; $85B38A
    sta.w $016E                 ; $85B38D
    lda.w $0160                 ; $85B390
    sta.w $0170                 ; $85B393
    lda.w $0162                 ; $85B396
    sta.w $0172                 ; $85B399
    lda.w $00E0                 ; $85B39C
    sta.w $01DC                 ; $85B39F
    sta.w $01DE                 ; $85B3A2
    jsr.w $B1FF                 ; $85B3A5
    rtl                         ; $85B3A8
    jsl.l $85B3D7               ; $85B3A9
    jsl.l $85B828               ; $85B3AD
    jsl.l $85BC20               ; $85B3B1
    lda.w $00D0                 ; $85B3B5
    sta.w $1E46                 ; $85B3B8
    lda.w $00D2                 ; $85B3BB
    sta.w $1E48                 ; $85B3BE
    lda.w $015C                 ; $85B3C1
    sta.w $016C                 ; $85B3C4
    lda.w $0160                 ; $85B3C7
    sta.w $0170                 ; $85B3CA
    lda.w $00E0                 ; $85B3CD
    sta.w $01DC                 ; $85B3D0
    jsr.w $B1FF                 ; $85B3D3
    rtl                         ; $85B3D6
    pea.w $8330                 ; $85B3D7
    plb                         ; $85B3DA
    ldy.w #$0023                ; $85B3DB
    ldx.w #$272B                ; $85B3DE
    lda.w #$06BC                ; $85B3E1
    sta.b $1A                   ; $85B3E4
    lda.w #$0004                ; $85B3E6
    sta.b $1C                   ; $85B3E9
    lda.w $00B4                 ; $85B3EB
    bne CODE_85B400             ; $85B3EE
    ldy.w #$0045                ; $85B3F0
    ldx.w #$282B                ; $85B3F3
    lda.w #$0D7C                ; $85B3F6
    sta.b $1A                   ; $85B3F9
    lda.w #$01A4                ; $85B3FB
    sta.b $1C                   ; $85B3FE
CODE_85B400:
    stx.b $18                   ; $85B400
    lda.w $0225                 ; $85B402
    lsr a                       ; $85B405
    and.w #$007E                ; $85B406
    eor.w #$FFFF                ; $85B409
    sec                         ; $85B40C
    adc.w #$0080                ; $85B40D
    sta.b $14                   ; $85B410
    lda.w $0226                 ; $85B412
    asl a                       ; $85B415
    and.w #$003E                ; $85B416
    sta.b $16                   ; $85B419
    ldx.b $16                   ; $85B41B
    stz.w $8000                 ; $85B41D
    lda.l $7E3AEC,x             ; $85B420
    sta.w $8000                 ; $85B424
    lda.w #$FC00                ; $85B427
    sta.w $8000                 ; $85B42A
    nop                         ; $85B42D
    lda.w $8000                 ; $85B42E
    clc                         ; $85B431
    adc.w #$8000                ; $85B432
    lda.w $8000                 ; $85B435
    adc.w #$0000                ; $85B438
    sta.w $0174                 ; $85B43B
    stz.w $8000                 ; $85B43E
    lda.l $7E3B6E,x             ; $85B441
    sta.w $8000                 ; $85B445
    lda.w #$0080                ; $85B448
    sec                         ; $85B44B
    sbc.b $14                   ; $85B44C
    lsr a                       ; $85B44E
    xba                         ; $85B44F
    sta.w $8000                 ; $85B450
    nop                         ; $85B453
    lda.w $8000                 ; $85B454
    sta.b $1E                   ; $85B457
    lda.w $8000                 ; $85B459
    asl.b $1E                   ; $85B45C
    rol a                       ; $85B45E
    asl.b $1E                   ; $85B45F
    rol a                       ; $85B461
    eor.w #$FFFF                ; $85B462
    inc a                       ; $85B465
    sta.b $1E                   ; $85B466
    lda.w $0208                 ; $85B468
    clc                         ; $85B46B
    adc.b $1E                   ; $85B46C
    sta.b $1E                   ; $85B46E
    stz.w $8000                 ; $85B470
    lda.w $0208                 ; $85B473
    sta.w $8000                 ; $85B476
    lda.b $14                   ; $85B479
    lsr a                       ; $85B47B
    xba                         ; $85B47C
    sta.w $8000                 ; $85B47D
    nop                         ; $85B480
    lda.w $8000                 ; $85B481
    sta.b $20                   ; $85B484
    lda.w $8000                 ; $85B486
    asl.b $20                   ; $85B489
    rol a                       ; $85B48B
    asl.b $20                   ; $85B48C
    rol a                       ; $85B48E
    sta.b $20                   ; $85B48F
    stz.w $8000                 ; $85B491
    lda.l $7E3AEE,x             ; $85B494
    sta.w $8000                 ; $85B498
    lda.w #$0080                ; $85B49B
    sec                         ; $85B49E
    sbc.b $14                   ; $85B49F
    lsr a                       ; $85B4A1
    xba                         ; $85B4A2
    sta.w $8000                 ; $85B4A3
    ldx.w #$0000                ; $85B4A6
    lda.w $8000                 ; $85B4A9
    sta.b $26                   ; $85B4AC
    lda.w $8000                 ; $85B4AE
    asl.b $26                   ; $85B4B1
    rol a                       ; $85B4B3
    asl.b $26                   ; $85B4B4
    rol a                       ; $85B4B6
    eor.w #$FFFF                ; $85B4B7
    inc a                       ; $85B4BA
    sta.b $26                   ; $85B4BB
    bpl CODE_85B4C0             ; $85B4BD
    dex                         ; $85B4BF
CODE_85B4C0:
    stx.b $28                   ; $85B4C0
    lda.w $020B                 ; $85B4C2
    adc.b $26                   ; $85B4C5
    sta.b $26                   ; $85B4C7
    lda.w $020D                 ; $85B4C9
    and.w #$00FF                ; $85B4CC
    bit.w #$0080                ; $85B4CF
    beq CODE_85B4D7             ; $85B4D2
    incbin !baserom:$2B4D4..$2B4D7
CODE_85B4D7:
    adc.b $28                   ; $85B4D7
    sta.b $28                   ; $85B4D9
    stz.w $8000                 ; $85B4DB
    lda.b $26                   ; $85B4DE
    sta.w $8000                 ; $85B4E0
    lda.b $14                   ; $85B4E3
    lsr a                       ; $85B4E5
    xba                         ; $85B4E6
    sta.w $8000                 ; $85B4E7
    ldx.w #$0000                ; $85B4EA
    lda.w $8000                 ; $85B4ED
    lda.w $8000                 ; $85B4F0
    sta.b $2A                   ; $85B4F3
    bpl CODE_85B4F8             ; $85B4F5
    dex                         ; $85B4F7
CODE_85B4F8:
    stx.b $2C                   ; $85B4F8
    stz.w $8000                 ; $85B4FA
    lda.b $28                   ; $85B4FD
    bit.b $26                   ; $85B4FF
    bpl CODE_85B504             ; $85B501
    inc a                       ; $85B503
CODE_85B504:
    sta.w $8000                 ; $85B504
    lda.b $14                   ; $85B507
    lsr a                       ; $85B509
    xba                         ; $85B50A
    sta.w $8000                 ; $85B50B
    nop                         ; $85B50E
    lda.w $8000                 ; $85B50F
    clc                         ; $85B512
    adc.b $2A                   ; $85B513
    sta.b $2A                   ; $85B515
    lda.w $8000                 ; $85B517
    adc.b $2C                   ; $85B51A
    asl.b $2A                   ; $85B51C
    rol a                       ; $85B51E
    asl.b $2A                   ; $85B51F
    rol a                       ; $85B521
    sta.b $2C                   ; $85B522
    tyx                         ; $85B524
    lda.w #$0000                ; $85B525
    sta.l $7E4CFD,x             ; $85B528
    lda.w #$0001                ; $85B52C
    bit.b $E4                   ; $85B52F
    bmi CODE_85B53B             ; $85B531
    incbin !baserom:$2B533..$2B53B
CODE_85B53B:
    sta.w $8000                 ; $85B53B
    nop                         ; $85B53E
    nop                         ; $85B53F
    lda.w #$0000                ; $85B540
    sta.w $8000                 ; $85B543
    lda.w $0242                 ; $85B546
    bpl CODE_85B558             ; $85B549
    incbin !baserom:$2B54B..$2B558
CODE_85B558:
    clc                         ; $85B558
    adc.w #$0033                ; $85B559
    sta.w $8000                 ; $85B55C
    lda.w #$00DC                ; $85B55F
    sta.l $7E4CFD,x             ; $85B562
    dex                         ; $85B566
    lda.w $0242                 ; $85B567
    bpl CODE_85B572             ; $85B56A
    incbin !baserom:$2B56C..$2B572
CODE_85B572:
    lda.w #$0033                ; $85B572
    sta.w $8000                 ; $85B575
    lda.w #$FF5B                ; $85B578
    sta.w $8000                 ; $85B57B
    lda.w #$00BF                ; $85B57E
    sta.w $8000                 ; $85B581
    lda.w $0242                 ; $85B584
    bpl CODE_85B58F             ; $85B587
    incbin !baserom:$2B589..$2B58F
CODE_85B58F:
    lda.w #$00DC                ; $85B58F
    sta.w $8000                 ; $85B592
    lda.w $0200                 ; $85B595
    sta.w $8000                 ; $85B598
    lda.w $0202                 ; $85B59B
    beq CODE_85B5A5             ; $85B59E
    dec a                       ; $85B5A0
    bpl CODE_85B5A5             ; $85B5A1
    inc a                       ; $85B5A3
    inc a                       ; $85B5A4
CODE_85B5A5:
    sta.w $8000                 ; $85B5A5
    lda.w #$0180                ; $85B5A8
    sta.w $8000                 ; $85B5AB
    lda.w $0242                 ; $85B5AE
    bpl CODE_85B5BA             ; $85B5B1
    incbin !baserom:$2B5B3..$2B5BA
CODE_85B5BA:
    lda.b $1A                   ; $85B5BA
    sta.w $8000                 ; $85B5BC
    lda.w #$00C0                ; $85B5BF
    sta.w $8000                 ; $85B5C2
    stx.b $22                   ; $85B5C5
    lda.b $1D                   ; $85B5C7
    and.w #$FF00                ; $85B5C9
    sta.w $8000                 ; $85B5CC
    lda.b $1F                   ; $85B5CF
    and.w #$00FF                ; $85B5D1
    bit.w #$0080                ; $85B5D4
    beq CODE_85B5DC             ; $85B5D7
    ora.w #$FF00                ; $85B5D9
CODE_85B5DC:
    sta.w $8000                 ; $85B5DC
    lda.w $020A                 ; $85B5DF
    and.w #$00FF                ; $85B5E2
    pha                         ; $85B5E5
    lda.b $25                   ; $85B5E6
    and.w #$FF00                ; $85B5E8
    ora.b $01,s                 ; $85B5EB
    sta.w $8000                 ; $85B5ED
    lda.b $27                   ; $85B5F0
    sta.w $8000                 ; $85B5F2
    pla                         ; $85B5F5
    lda.w #$0002                ; $85B5F6
    sta.b $24                   ; $85B5F9
    ldx.b $14                   ; $85B5FB
    lda.l $83ACEC,x             ; $85B5FD
    sta.w $8000                 ; $85B601
    sta.b $30                   ; $85B604
    lda.b $20                   ; $85B606
    sta.w $8000                 ; $85B608
    lda.b $29                   ; $85B60B
    and.w #$FF00                ; $85B60D
    sta.w $8000                 ; $85B610
    lda.b $2B                   ; $85B613
    sta.w $8000                 ; $85B615
    jmp.w $B624                 ; $85B618
    lda.l $83ACEC,x             ; $85B61B
    sta.w $8000                 ; $85B61F
    sta.b $30                   ; $85B622
    ldx.b $16                   ; $85B624
    lda.l $7E3B6E,x             ; $85B626
    sta.w $8000                 ; $85B62A
    lda.l $7E3AEE,x             ; $85B62D
    sta.w $8000                 ; $85B631
    lda.l $7E3BEC,x             ; $85B634
    sta.w $8000                 ; $85B638
    inx                         ; $85B63B
    inx                         ; $85B63C
    stx.b $16                   ; $85B63D
    ldx.b $24                   ; $85B63F
    lda.w $8000                 ; $85B641
    sta.l $7E34EC,x             ; $85B644
    lda.w $8000                 ; $85B648
    sta.l $7E35EC,x             ; $85B64B
    lda.w $8000                 ; $85B64F
    sta.l $7E36EC,x             ; $85B652
    lda.w $8000                 ; $85B656
    sta.l $7E37EC,x             ; $85B659
    lda.b $30                   ; $85B65D
    sta.l $7E38EC,x             ; $85B65F
    lda.b $14                   ; $85B663
    sta.l $7E39EC,x             ; $85B665
    inx                         ; $85B669
    inx                         ; $85B66A
    stx.b $24                   ; $85B66B
CODE_85B66D:
    lda.w $C000                 ; $85B66D
    bpl CODE_85B66D             ; $85B670
    lda.w $8000                 ; $85B672
    bne CODE_85B67A             ; $85B675
    jmp.w $B755                 ; $85B677
CODE_85B67A:
    tay                         ; $85B67A
    ldx.b $22                   ; $85B67B
    sep.b #$20                  ; $85B67D
    eor.b #$FF                  ; $85B67F
    sec                         ; $85B681
    adc.l $7E4CFE,x             ; $85B682
    sta.l $7E4CFD,x             ; $85B686
    rep.b #$20                  ; $85B68A
    phy                         ; $85B68C
    lda.b $E4                   ; $85B68D
    db $10,$03 ; bpl            ; $85B68F
    jmp.w $B716                 ; $85B691
    incbin !baserom:$2B694..$2B716
    ldx.b $16                   ; $85B716
    lda.l $7E3CEA,x             ; $85B718
    tax                         ; $85B71C
    phb                         ; $85B71D
    ldy.b $18                   ; $85B71E
    lda.w #$0007                ; $85B720
    mvp $7E,$90                 ; $85B723
    sty.b $18                   ; $85B726
    plb                         ; $85B728
    ply                         ; $85B729
CODE_85B72A:
    lda.w $C000                 ; $85B72A
    bpl CODE_85B72A             ; $85B72D
    dey                         ; $85B72F
    beq CODE_85B744             ; $85B730
CODE_85B732:
    ldx.w $8000                 ; $85B732
    lda.w $8000                 ; $85B735
    sta.w $0798,x               ; $85B738
    lda.w $8000                 ; $85B73B
    sta.w $0796,x               ; $85B73E
    dey                         ; $85B741
    bne CODE_85B732             ; $85B742
CODE_85B744:
    ldx.w $8000                 ; $85B744
    lda.w $8000                 ; $85B747
    sta.w $0798,x               ; $85B74A
    lda.w $8000                 ; $85B74D
    sta.w $0796,x               ; $85B750
    dec.b $22                   ; $85B753
    ldx.b $16                   ; $85B755
    lda.l $7E3D6A,x             ; $85B757
    bpl CODE_85B7D6             ; $85B75B
    incbin !baserom:$2B75D..$2B7D6
CODE_85B7D6:
    lda.b $14                   ; $85B7D6
    clc                         ; $85B7D8
    adc.w #$0080                ; $85B7D9
    sta.b $14                   ; $85B7DC
    cmp.w #$0E00                ; $85B7DE
    bcs CODE_85B7E7             ; $85B7E1
    tax                         ; $85B7E3
    jmp.w $B61B                 ; $85B7E4
CODE_85B7E7:
    lda.w #$8000                ; $85B7E7
    sta.w $8000                 ; $85B7EA
    lda.b $24                   ; $85B7ED
    sta.b $90                   ; $85B7EF
    ldx.w #$0023                ; $85B7F1
    lda.w $00B4                 ; $85B7F4
    bne CODE_85B7FC             ; $85B7F7
    ldx.w #$0045                ; $85B7F9
CODE_85B7FC:
    ldy.w #$0000                ; $85B7FC
    lda.w $0242                 ; $85B7FF
    bpl CODE_85B811             ; $85B802
    incbin !baserom:$2B804..$2B811
CODE_85B811:
    tya                         ; $85B811
    sta.l $7E4CFD,x             ; $85B812
    lda.b $22                   ; $85B816
    sta.w $0164                 ; $85B818
    ldy.b $18                   ; $85B81B
    iny                         ; $85B81D
    sty.w $015C                 ; $85B81E
    lda.b $1A                   ; $85B821
    sta.w $1E3E                 ; $85B823
    plb                         ; $85B826
    rtl                         ; $85B827
    pea.w $8330                 ; $85B828
    plb                         ; $85B82B
    ldy.w #$0023                ; $85B82C
    ldx.w #$292B                ; $85B82F
    lda.w #$035C                ; $85B832
    sta.b $1A                   ; $85B835
    lda.w #$0004                ; $85B837
    sta.b $1C                   ; $85B83A
    lda.w $00B4                 ; $85B83C
    bne CODE_85B851             ; $85B83F
    ldy.w #$0045                ; $85B841
    ldx.w #$2A2B                ; $85B844
    lda.w #$050C                ; $85B847
    sta.b $1A                   ; $85B84A
    lda.w #$01A4                ; $85B84C
    sta.b $1C                   ; $85B84F
CODE_85B851:
    stx.b $18                   ; $85B851
    lda.w $0225                 ; $85B853
    lsr a                       ; $85B856
    and.w #$007E                ; $85B857
    eor.w #$FFFF                ; $85B85A
    sec                         ; $85B85D
    adc.w #$0080                ; $85B85E
    sta.b $14                   ; $85B861
    lda.w $0226                 ; $85B863
    asl a                       ; $85B866
    and.w #$003E                ; $85B867
    sta.b $16                   ; $85B86A
    ldx.b $16                   ; $85B86C
    lda.l $7E3E6C,x             ; $85B86E
    sta.l $7E376C               ; $85B872
    lda.l $7E3DEC,x             ; $85B876
    sta.l $7E356C               ; $85B87A
    stz.w $8000                 ; $85B87E
    lda.l $7E3E6E,x             ; $85B881
    sec                         ; $85B885
    sbc.l $7E3E6C,x             ; $85B886
    asl a                       ; $85B88A
    sta.w $8000                 ; $85B88B
    lda.w $0224                 ; $85B88E
    lsr a                       ; $85B891
    and.w #$FF00                ; $85B892
    sta.w $8000                 ; $85B895
    nop                         ; $85B898
    lda.w $8000                 ; $85B899
    lda.w $0242                 ; $85B89C
    clc                         ; $85B89F
    adc.w $8000                 ; $85B8A0
    clc                         ; $85B8A3
    adc.l $7E3E6C,x             ; $85B8A4
    sta.w $1EF2                 ; $85B8A8
    stz.w $8000                 ; $85B8AB
    lda.l $7E3AEC,x             ; $85B8AE
    sta.w $8000                 ; $85B8B2
    lda.w #$FC00                ; $85B8B5
    sta.w $8000                 ; $85B8B8
    nop                         ; $85B8BB
    lda.w $8000                 ; $85B8BC
    clc                         ; $85B8BF
    adc.w #$8000                ; $85B8C0
    lda.w $8000                 ; $85B8C3
    adc.w #$0000                ; $85B8C6
    sta.w $017C                 ; $85B8C9
    stz.w $8000                 ; $85B8CC
    lda.l $7E3B6E,x             ; $85B8CF
    sta.w $8000                 ; $85B8D3
    lda.w #$0080                ; $85B8D6
    sec                         ; $85B8D9
    sbc.b $14                   ; $85B8DA
    lsr a                       ; $85B8DC
    xba                         ; $85B8DD
    sta.w $8000                 ; $85B8DE
    nop                         ; $85B8E1
    lda.w $8000                 ; $85B8E2
    sta.b $1E                   ; $85B8E5
    lda.w $8000                 ; $85B8E7
    asl.b $1E                   ; $85B8EA
    rol a                       ; $85B8EC
    asl.b $1E                   ; $85B8ED
    rol a                       ; $85B8EF
    eor.w #$FFFF                ; $85B8F0
    inc a                       ; $85B8F3
    sta.b $1E                   ; $85B8F4
    lda.w $1EDA                 ; $85B8F6
    clc                         ; $85B8F9
    adc.b $1E                   ; $85B8FA
    sta.b $1E                   ; $85B8FC
    stz.w $8000                 ; $85B8FE
    lda.l $7E3AEE,x             ; $85B901
    sta.w $8000                 ; $85B905
    lda.w #$0080                ; $85B908
    sec                         ; $85B90B
    sbc.b $14                   ; $85B90C
    lsr a                       ; $85B90E
    xba                         ; $85B90F
    sta.w $8000                 ; $85B910
    nop                         ; $85B913
    lda.w $8000                 ; $85B914
    sta.b $26                   ; $85B917
    lda.w $8000                 ; $85B919
    asl.b $26                   ; $85B91C
    rol a                       ; $85B91E
    asl.b $26                   ; $85B91F
    rol a                       ; $85B921
    eor.w #$FFFF                ; $85B922
    inc a                       ; $85B925
    sta.b $26                   ; $85B926
    stz.w $8000                 ; $85B928
    lda.b $26                   ; $85B92B
    sta.w $8000                 ; $85B92D
    lda.b $14                   ; $85B930
    lsr a                       ; $85B932
    xba                         ; $85B933
    sta.w $8000                 ; $85B934
    nop                         ; $85B937
    lda.w $8000                 ; $85B938
    sta.b $2A                   ; $85B93B
    lda.w $8000                 ; $85B93D
    asl.b $2A                   ; $85B940
    rol a                       ; $85B942
    asl.b $2A                   ; $85B943
    rol a                       ; $85B945
    sta.b $2A                   ; $85B946
    tyx                         ; $85B948
    lda.w #$0000                ; $85B949
    sta.l $7E4D49,x             ; $85B94C
    lda.w #$0007                ; $85B950
    bit.b $E4                   ; $85B953
    bmi CODE_85B95F             ; $85B955
    incbin !baserom:$2B957..$2B95F
CODE_85B95F:
    sta.w $8000                 ; $85B95F
    nop                         ; $85B962
    nop                         ; $85B963
    lda.w #$0000                ; $85B964
    sta.w $8000                 ; $85B967
    lda.w $1EF2                 ; $85B96A
    bpl CODE_85B97C             ; $85B96D
    incbin !baserom:$2B96F..$2B97C
CODE_85B97C:
    clc                         ; $85B97C
    adc.w #$0033                ; $85B97D
    sta.w $8000                 ; $85B980
    lda.w #$00DC                ; $85B983
    sta.l $7E4D49,x             ; $85B986
    dex                         ; $85B98A
    lda.w $1EF2                 ; $85B98B
    bpl CODE_85B996             ; $85B98E
    incbin !baserom:$2B990..$2B996
CODE_85B996:
    lda.w #$0033                ; $85B996
    sta.w $8000                 ; $85B999
    lda.w #$FF5B                ; $85B99C
    sta.w $8000                 ; $85B99F
    lda.w #$00BF                ; $85B9A2
    sta.w $8000                 ; $85B9A5
    lda.w $1EF2                 ; $85B9A8
    bpl CODE_85B9B3             ; $85B9AB
    incbin !baserom:$2B9AD..$2B9B3
CODE_85B9B3:
    lda.w #$00DC                ; $85B9B3
    sta.w $8000                 ; $85B9B6
    lda.w $1EF4                 ; $85B9B9
    sta.w $8000                 ; $85B9BC
    lda.w $1EF6                 ; $85B9BF
    sta.w $8000                 ; $85B9C2
    lda.w #$0180                ; $85B9C5
    sta.w $8000                 ; $85B9C8
    lda.w $1EF2                 ; $85B9CB
    bpl CODE_85B9D7             ; $85B9CE
    incbin !baserom:$2B9D0..$2B9D7
CODE_85B9D7:
    lda.b $1A                   ; $85B9D7
    sta.w $8000                 ; $85B9D9
    lda.w #$00C0                ; $85B9DC
    sta.w $8000                 ; $85B9DF
    stx.b $22                   ; $85B9E2
    lda.w #$0002                ; $85B9E4
    sta.b $24                   ; $85B9E7
    ldx.b $14                   ; $85B9E9
    lda.l $83ACEC,x             ; $85B9EB
    sta.w $8000                 ; $85B9EF
    sta.b $30                   ; $85B9F2
    ldx.b $24                   ; $85B9F4
    lda.l $7E37EC,x             ; $85B9F6
    sta.w $8000                 ; $85B9FA
    ldx.b $16                   ; $85B9FD
    lda.l $7E3E6E,x             ; $85B9FF
    sta.w $8000                 ; $85BA03
    ldx.b $24                   ; $85BA06
    lda.l $7E35EC,x             ; $85BA08
    sta.w $8000                 ; $85BA0C
    ldx.b $16                   ; $85BA0F
    lda.l $7E3DEE,x             ; $85BA11
    sta.w $8000                 ; $85BA15
    ldx.b $16                   ; $85BA18
    lda.l $7E3BEC,x             ; $85BA1A
    sta.w $8000                 ; $85BA1E
    inx                         ; $85BA21
    inx                         ; $85BA22
    stx.b $16                   ; $85BA23
    lda.l $7E3E6C,x             ; $85BA25
    clc                         ; $85BA29
    ldx.b $24                   ; $85BA2A
    adc.l $7E36EC,x             ; $85BA2C
    sta.l $7E376C,x             ; $85BA30
    ldx.b $16                   ; $85BA34
    lda.l $7E3DEC,x             ; $85BA36
    clc                         ; $85BA3A
    ldx.b $24                   ; $85BA3B
    adc.l $7E34EC,x             ; $85BA3D
    sta.l $7E356C,x             ; $85BA41
    ldx.b $24                   ; $85BA45
    lda.w $8000                 ; $85BA47
    sta.l $7E366C,x             ; $85BA4A
    lda.w $8000                 ; $85BA4E
    sta.l $7E386C,x             ; $85BA51
    lda.b $30                   ; $85BA55
    sta.l $7E396C,x             ; $85BA57
    lda.b $14                   ; $85BA5B
    sta.l $7E3A6C,x             ; $85BA5D
    inx                         ; $85BA61
    inx                         ; $85BA62
    stx.b $24                   ; $85BA63
CODE_85BA65:
    lda.w $C000                 ; $85BA65
    bpl CODE_85BA65             ; $85BA68
    lda.w $8000                 ; $85BA6A
    bne CODE_85BA72             ; $85BA6D
    jmp.w $BBCE                 ; $85BA6F
CODE_85BA72:
    tay                         ; $85BA72
    ldx.b $22                   ; $85BA73
    sep.b #$20                  ; $85BA75
    eor.b #$FF                  ; $85BA77
    sec                         ; $85BA79
    adc.l $7E4D4A,x             ; $85BA7A
    sta.l $7E4D49,x             ; $85BA7E
    rep.b #$20                  ; $85BA82
    phy                         ; $85BA84
    lda.b $E4                   ; $85BA85
    db $10,$03 ; bpl            ; $85BA87
    jmp.w $BB7F                 ; $85BA89
    incbin !baserom:$2BA8C..$2BB7F
    ldx.b $16                   ; $85BB7F
    lda.l $7E3FEC,x             ; $85BB81
    tax                         ; $85BB85
    bmi CODE_85BB96             ; $85BB86
    phb                         ; $85BB88
    ldy.b $18                   ; $85BB89
    lda.w #$0007                ; $85BB8B
    mvp $7E,$7E                 ; $85BB8E
    sty.b $18                   ; $85BB91
    plb                         ; $85BB93
    bra CODE_85BBA2             ; $85BB94
CODE_85BB96:
    phb                         ; $85BB96
    ldy.b $18                   ; $85BB97
    lda.w #$0007                ; $85BB99
    mvp $7E,$90                 ; $85BB9C
    sty.b $18                   ; $85BB9F
    plb                         ; $85BBA1
CODE_85BBA2:
    ply                         ; $85BBA2
CODE_85BBA3:
    lda.w $C000                 ; $85BBA3
    bpl CODE_85BBA3             ; $85BBA6
    dey                         ; $85BBA8
    beq CODE_85BBBD             ; $85BBA9
CODE_85BBAB:
    ldx.w $8000                 ; $85BBAB
    lda.w $8000                 ; $85BBAE
    sta.w $0E58,x               ; $85BBB1
    lda.w $8000                 ; $85BBB4
    sta.w $0E56,x               ; $85BBB7
    dey                         ; $85BBBA
    bne CODE_85BBAB             ; $85BBBB
CODE_85BBBD:
    ldx.w $8000                 ; $85BBBD
    lda.w $8000                 ; $85BBC0
    sta.w $0E58,x               ; $85BBC3
    lda.w $8000                 ; $85BBC6
    sta.w $0E56,x               ; $85BBC9
    dec.b $22                   ; $85BBCC
    lda.b $14                   ; $85BBCE
    clc                         ; $85BBD0
    adc.w #$0080                ; $85BBD1
    sta.b $14                   ; $85BBD4
    cmp.w #$0E00                ; $85BBD6
    bcs CODE_85BBDF             ; $85BBD9
    tax                         ; $85BBDB
    jmp.w $B9EB                 ; $85BBDC
CODE_85BBDF:
    lda.w #$8000                ; $85BBDF
    sta.w $8000                 ; $85BBE2
    lda.b $24                   ; $85BBE5
    sta.b $94                   ; $85BBE7
    ldx.w #$0023                ; $85BBE9
    lda.w $00B4                 ; $85BBEC
    bne CODE_85BBF4             ; $85BBEF
    ldx.w #$0045                ; $85BBF1
CODE_85BBF4:
    ldy.w #$0000                ; $85BBF4
    lda.w $1EF2                 ; $85BBF7
    bpl CODE_85BC09             ; $85BBFA
    incbin !baserom:$2BBFC..$2BC09
CODE_85BC09:
    tya                         ; $85BC09
    sta.l $7E4D49,x             ; $85BC0A
    lda.b $22                   ; $85BC0E
    sta.w $0168                 ; $85BC10
    ldy.b $18                   ; $85BC13
    iny                         ; $85BC15
    sty.w $0160                 ; $85BC16
    lda.b $1A                   ; $85BC19
    sta.w $1E40                 ; $85BC1B
    plb                         ; $85BC1E
    rtl                         ; $85BC1F
    lda.w $00E0                 ; $85BC20
    ldx.w $00E4                 ; $85BC23
    sta.w $00E4                 ; $85BC26
    stx.w $00E0                 ; $85BC29
    ldx.w $1E36                 ; $85BC2C
    lda.w $1E32                 ; $85BC2F
    sta.w $1E36                 ; $85BC32
    sta.w $00D0                 ; $85BC35
    lda.w $1E2E                 ; $85BC38
    sta.w $1E32                 ; $85BC3B
    stx.w $1E2E                 ; $85BC3E
    ldx.w $1E38                 ; $85BC41
    lda.w $1E34                 ; $85BC44
    sta.w $1E38                 ; $85BC47
    sta.w $00D2                 ; $85BC4A
    lda.w $1E30                 ; $85BC4D
    sta.w $1E34                 ; $85BC50
    stx.w $1E30                 ; $85BC53
    ldy.w $00D0                 ; $85BC56
    lda.w #$0000                ; $85BC59
    sta.w $0000,y               ; $85BC5C
    dey                         ; $85BC5F
    dey                         ; $85BC60
    dey                         ; $85BC61
    lda.w #$00D8                ; $85BC62
    sta.b $14                   ; $85BC65
    lda.w $0242                 ; $85BC67
    bpl CODE_85BCAB             ; $85BC6A
    incbin !baserom:$2BC6C..$2BCAB
CODE_85BCAB:
    lda.b $14                   ; $85BCAB
    sta.b $16                   ; $85BCAD
    ldx.w $0164                 ; $85BCAF
    lda.l $7E4CFE,x             ; $85BCB2
    and.w #$00FF                ; $85BCB6
    bne CODE_85BCBE             ; $85BCB9
    incbin !baserom:$2BCBB..$2BCBE
CODE_85BCBE:
    sec                         ; $85BCBE
    sbc.w #$0004                ; $85BCBF
    bpl CODE_85BCC7             ; $85BCC2
    incbin !baserom:$2BCC4..$2BCC7
CODE_85BCC7:
    cmp.b $14                   ; $85BCC7
    bpl CODE_85BCF4             ; $85BCC9
    sta.b $14                   ; $85BCCB
    eor.w #$FFFF                ; $85BCCD
    sec                         ; $85BCD0
    adc.b $16                   ; $85BCD1
    beq CODE_85BCF4             ; $85BCD3
    ora.w #$0080                ; $85BCD5
    sta.w $0000,y               ; $85BCD8
    lda.w #$00D7                ; $85BCDB
    sec                         ; $85BCDE
    sbc.b $14                   ; $85BCDF
    asl a                       ; $85BCE1
    asl a                       ; $85BCE2
    eor.w #$FFFF                ; $85BCE3
    sec                         ; $85BCE6
    adc.w $1E3E                 ; $85BCE7
    clc                         ; $85BCEA
    adc.w #$0796                ; $85BCEB
    sta.w $0001,y               ; $85BCEE
    dey                         ; $85BCF1
    dey                         ; $85BCF2
    dey                         ; $85BCF3
CODE_85BCF4:
    lda.w $0237                 ; $85BCF4
    clc                         ; $85BCF7
    adc.w #$00B0                ; $85BCF8
    bpl CODE_85BD00             ; $85BCFB
    incbin !baserom:$2BCFD..$2BD00
CODE_85BD00:
    sta.b $16                   ; $85BD00
    sec                         ; $85BD02
    sbc.b $14                   ; $85BD03
    bpl CODE_85BD21             ; $85BD05
    eor.w #$FFFF                ; $85BD07
    inc a                       ; $85BD0A
    ora.w #$0080                ; $85BD0B
    sta.w $0000,y               ; $85BD0E
    lda.b $16                   ; $85BD11
    sta.b $14                   ; $85BD13
    asl a                       ; $85BD15
    asl a                       ; $85BD16
    clc                         ; $85BD17
    adc.w #$A968                ; $85BD18
    sta.w $0001,y               ; $85BD1B
    dey                         ; $85BD1E
    dey                         ; $85BD1F
    dey                         ; $85BD20
CODE_85BD21:
    lda.w $0237                 ; $85BD21
    clc                         ; $85BD24
    adc.w #$0070                ; $85BD25
    bpl CODE_85BD2D             ; $85BD28
    incbin !baserom:$2BD2A..$2BD2D
CODE_85BD2D:
    sta.b $16                   ; $85BD2D
    sec                         ; $85BD2F
    sbc.b $14                   ; $85BD30
    bpl CODE_85BD4C             ; $85BD32
    eor.w #$FFFF                ; $85BD34
    inc a                       ; $85BD37
    sta.w $0000,y               ; $85BD38
    lda.w $00E0                 ; $85BD3B
    clc                         ; $85BD3E
    adc.w #$1536                ; $85BD3F
    sta.w $0001,y               ; $85BD42
    dey                         ; $85BD45
    dey                         ; $85BD46
    dey                         ; $85BD47
    lda.b $16                   ; $85BD48
    sta.b $14                   ; $85BD4A
CODE_85BD4C:
    lda.b $14                   ; $85BD4C
    beq CODE_85BD79             ; $85BD4E
    cmp.w #$0080                ; $85BD50
    bcc CODE_85BD6A             ; $85BD53
    sbc.w #$007F                ; $85BD55
    ora.w #$0080                ; $85BD58
    sta.w $0000,y               ; $85BD5B
    lda.w #$A7D8                ; $85BD5E
    sta.w $0001,y               ; $85BD61
    dey                         ; $85BD64
    dey                         ; $85BD65
    dey                         ; $85BD66
    lda.w #$007F                ; $85BD67
CODE_85BD6A:
    ora.w #$0080                ; $85BD6A
    sta.w $0000,y               ; $85BD6D
    lda.w #$A5DC                ; $85BD70
    sta.w $0001,y               ; $85BD73
    dey                         ; $85BD76
    dey                         ; $85BD77
    dey                         ; $85BD78
CODE_85BD79:
    lda.w #$0004                ; $85BD79
    sta.w $0000,y               ; $85BD7C
    sty.w $00D0                 ; $85BD7F
    ldy.w $00E0                 ; $85BD82
    lda.w #$00BB                ; $85BD85
    sec                         ; $85BD88
    sbc.w $0237                 ; $85BD89
    sec                         ; $85BD8C
    sbc.w #$0070                ; $85BD8D
    sta.w $1538,y               ; $85BD90
    ldy.w $00D2                 ; $85BD93
    lda.w #$0000                ; $85BD96
    sta.w $0000,y               ; $85BD99
    dey                         ; $85BD9C
    dey                         ; $85BD9D
    dey                         ; $85BD9E
    lda.w #$00D8                ; $85BD9F
    sta.b $14                   ; $85BDA2
    lda.w $1EF2                 ; $85BDA4
    bpl CODE_85BDE8             ; $85BDA7
    incbin !baserom:$2BDA9..$2BDE8
CODE_85BDE8:
    lda.b $14                   ; $85BDE8
    sta.b $16                   ; $85BDEA
    ldx.w $0168                 ; $85BDEC
    lda.l $7E4D4A,x             ; $85BDEF
    and.w #$00FF                ; $85BDF3
    bne CODE_85BDFB             ; $85BDF6
    incbin !baserom:$2BDF8..$2BDFB
CODE_85BDFB:
    sec                         ; $85BDFB
    sbc.w #$0004                ; $85BDFC
    bpl CODE_85BE04             ; $85BDFF
    incbin !baserom:$2BE01..$2BE04
CODE_85BE04:
    cmp.b $14                   ; $85BE04
    bpl CODE_85BE31             ; $85BE06
    sta.b $14                   ; $85BE08
    eor.w #$FFFF                ; $85BE0A
    sec                         ; $85BE0D
    adc.b $16                   ; $85BE0E
    beq CODE_85BE31             ; $85BE10
    ora.w #$0080                ; $85BE12
    sta.w $0000,y               ; $85BE15
    lda.w #$00D7                ; $85BE18
    sec                         ; $85BE1B
    sbc.b $14                   ; $85BE1C
    asl a                       ; $85BE1E
    asl a                       ; $85BE1F
    eor.w #$FFFF                ; $85BE20
    sec                         ; $85BE23
    adc.w $1E40                 ; $85BE24
    clc                         ; $85BE27
    adc.w #$0E56                ; $85BE28
    sta.w $0001,y               ; $85BE2B
    dey                         ; $85BE2E
    dey                         ; $85BE2F
    dey                         ; $85BE30
CODE_85BE31:
    lda.w $0237                 ; $85BE31
    clc                         ; $85BE34
    adc.w #$00B0                ; $85BE35
    bpl CODE_85BE3D             ; $85BE38
    incbin !baserom:$2BE3A..$2BE3D
CODE_85BE3D:
    sta.b $16                   ; $85BE3D
    sec                         ; $85BE3F
    sbc.b $14                   ; $85BE40
    bpl CODE_85BE5E             ; $85BE42
    eor.w #$FFFF                ; $85BE44
    inc a                       ; $85BE47
    ora.w #$0080                ; $85BE48
    sta.w $0000,y               ; $85BE4B
    lda.b $16                   ; $85BE4E
    sta.b $14                   ; $85BE50
    asl a                       ; $85BE52
    asl a                       ; $85BE53
    clc                         ; $85BE54
    adc.w #$A968                ; $85BE55
    sta.w $0001,y               ; $85BE58
    dey                         ; $85BE5B
    dey                         ; $85BE5C
    dey                         ; $85BE5D
CODE_85BE5E:
    lda.w $0237                 ; $85BE5E
    clc                         ; $85BE61
    adc.w #$0070                ; $85BE62
    bpl CODE_85BE6A             ; $85BE65
    incbin !baserom:$2BE67..$2BE6A
CODE_85BE6A:
    sta.b $16                   ; $85BE6A
    sec                         ; $85BE6C
    sbc.b $14                   ; $85BE6D
    bpl CODE_85BE89             ; $85BE6F
    eor.w #$FFFF                ; $85BE71
    inc a                       ; $85BE74
    sta.w $0000,y               ; $85BE75
    lda.w $00E0                 ; $85BE78
    clc                         ; $85BE7B
    adc.w #$1556                ; $85BE7C
    sta.w $0001,y               ; $85BE7F
    dey                         ; $85BE82
    dey                         ; $85BE83
    dey                         ; $85BE84
    lda.b $16                   ; $85BE85
    sta.b $14                   ; $85BE87
CODE_85BE89:
    lda.b $14                   ; $85BE89
    beq CODE_85BEB6             ; $85BE8B
    cmp.w #$0080                ; $85BE8D
    bcc CODE_85BEA7             ; $85BE90
    sbc.w #$007F                ; $85BE92
    ora.w #$0080                ; $85BE95
    sta.w $0000,y               ; $85BE98
    lda.w #$A7D8                ; $85BE9B
    sta.w $0001,y               ; $85BE9E
    dey                         ; $85BEA1
    dey                         ; $85BEA2
    dey                         ; $85BEA3
    lda.w #$007F                ; $85BEA4
CODE_85BEA7:
    ora.w #$0080                ; $85BEA7
    sta.w $0000,y               ; $85BEAA
    lda.w #$A5DC                ; $85BEAD
    sta.w $0001,y               ; $85BEB0
    dey                         ; $85BEB3
    dey                         ; $85BEB4
    dey                         ; $85BEB5
CODE_85BEB6:
    lda.w #$0004                ; $85BEB6
    sta.w $0000,y               ; $85BEB9
    sty.w $00D2                 ; $85BEBC
    ldy.w $00E0                 ; $85BEBF
    lda.w #$00BB                ; $85BEC2
    sec                         ; $85BEC5
    sbc.w $0237                 ; $85BEC6
    sec                         ; $85BEC9
    sbc.w #$0070                ; $85BECA
    sta.w $1558,y               ; $85BECD
    rtl                         ; $85BED0
    incbin !baserom:$2BED1..$2BF78
    pea.w $837E                 ; $85BF78
    plb                         ; $85BF7B
    lda.w $023E                 ; $85BF7C
    asl a                       ; $85BF7F
    and.w #$003E                ; $85BF80
    tax                         ; $85BF83
    lda.w $0226                 ; $85BF84
    sec                         ; $85BF87
    sbc.w $023E                 ; $85BF88
    db $F0,$79 ; beq            ; $85BF8B
    tay                         ; $85BF8D
    lda.w #$0000                ; $85BF8E
    sta.l $308000               ; $85BF91
    sec                         ; $85BF95
    sbc.w $023C                 ; $85BF96
    xba                         ; $85BF99
    and.w #$00FF                ; $85BF9A
    sta.l $308000               ; $85BF9D
    lda.w $3AEE,x               ; $85BFA1
    sta.l $308000               ; $85BFA4
    nop                         ; $85BFA8
    lda.l $308000               ; $85BFA9
    xba                         ; $85BFAD
    pha                         ; $85BFAE
    and.w #$FF00                ; $85BFAF
    sta.b $14                   ; $85BFB2
    pla                         ; $85BFB4
    and.w #$00FF                ; $85BFB5
    sta.b $16                   ; $85BFB8
    lda.l $308000               ; $85BFBA
    xba                         ; $85BFBE
    and.w #$FF00                ; $85BFBF
    ora.b $16                   ; $85BFC2
    sta.w $01CE                 ; $85BFC4
    inx                         ; $85BFC7
    inx                         ; $85BFC8
    dey                         ; $85BFC9
    beq CODE_85BFD7             ; $85BFCA
CODE_85BFCC:
    adc.w $3AEE,x               ; $85BFCC
    inx                         ; $85BFCF
    inx                         ; $85BFD0
    dey                         ; $85BFD1
    bne CODE_85BFCC             ; $85BFD2
    sta.w $01CE                 ; $85BFD4
CODE_85BFD7:
    lda.w #$0000                ; $85BFD7
    sta.l $308000               ; $85BFDA
    lda.w $0224                 ; $85BFDE
    and.w #$FF00                ; $85BFE1
    lsr a                       ; $85BFE4
    sta.l $308000               ; $85BFE5
    lda.w $3AEE,x               ; $85BFE9
    asl a                       ; $85BFEC
    sta.l $308000               ; $85BFED
    nop                         ; $85BFF1
    lda.l $308000               ; $85BFF2
    clc                         ; $85BFF6
    adc.b $14                   ; $85BFF7
    lda.l $308000               ; $85BFF9
    clc                         ; $85BFFD
    adc.w $01CE                 ; $85BFFE
    sta.w $01CE                 ; $85C001
    bra CODE_85C030             ; $85C004
    incbin !baserom:$2C006..$2C030
CODE_85C030:
    lda.w $023E                 ; $85C030
    clc                         ; $85C033
    adc.w #$0020                ; $85C034
    cmp.w $019A                 ; $85C037
    bcc CODE_85C03F             ; $85C03A
    incbin !baserom:$2C03C..$2C03F
CODE_85C03F:
    sta.b $16                   ; $85C03F
    lda.w $0224                 ; $85C041
    and.w #$FF00                ; $85C044
    sta.w $023C                 ; $85C047
    lda.w $0226                 ; $85C04A
    sec                         ; $85C04D
    sbc.w $023E                 ; $85C04E
    bne CODE_85C056             ; $85C051
    incbin !baserom:$2C053..$2C056
CODE_85C056:
    sta.b $14                   ; $85C056
    lda.w $0244                 ; $85C058
    lsr a                       ; $85C05B
    bcs CODE_85C067             ; $85C05C
    incbin !baserom:$2C05E..$2C067
CODE_85C067:
    lda.w $023E                 ; $85C067
    asl a                       ; $85C06A
    and.w #$003E                ; $85C06B
    tax                         ; $85C06E
    lda.l $7E3E6E,x             ; $85C06F
    sec                         ; $85C073
    sbc.l $7E3E6C,x             ; $85C074
    sta.w $1EF8                 ; $85C078
    lda.l $7E3DEE,x             ; $85C07B
    sec                         ; $85C07F
    sbc.l $7E3DEC,x             ; $85C080
    sta.w $1EFA                 ; $85C084
    lda.w #$0000                ; $85C087
    sta.l $308000               ; $85C08A
    lda.b $16                   ; $85C08E
    asl a                       ; $85C090
    asl a                       ; $85C091
    xba                         ; $85C092
    and.w #$00FF                ; $85C093
    sta.l $308000               ; $85C096
    lda.w #$000A                ; $85C09A
    sta.l $308000               ; $85C09D
    nop                         ; $85C0A1
    lda.l $308000               ; $85C0A2
    clc                         ; $85C0A6
    adc.b $98                   ; $85C0A7
    sta.b $18                   ; $85C0A9
    tax                         ; $85C0AB
    lda.l $308000               ; $85C0AC
    lda.b $16                   ; $85C0B0
    asl a                       ; $85C0B2
    and.w #$003E                ; $85C0B3
    tay                         ; $85C0B6
    sta.b $1A                   ; $85C0B7
    lda.b $16                   ; $85C0B9
    and.w #$003E                ; $85C0BB
    sta.b $1C                   ; $85C0BE
    adc.l $840000,x             ; $85C0C0
    tax                         ; $85C0C4
    lda.l $850000,x             ; $85C0C5
    sta.w $3AEC,y               ; $85C0C9
    sta.w $3B2C,y               ; $85C0CC
    ldx.b $18                   ; $85C0CF
    lda.b $1C                   ; $85C0D1
    clc                         ; $85C0D3
    adc.l $840002,x             ; $85C0D4
    tax                         ; $85C0D8
    lda.l $850000,x             ; $85C0D9
    sec                         ; $85C0DD
    sbc.w $1E1E                 ; $85C0DE
    sta.w $3B6C,y               ; $85C0E1
    sta.w $3BAC,y               ; $85C0E4
    tya                         ; $85C0E7
    and.w #$0006                ; $85C0E8
    cmp.w #$0006                ; $85C0EB
    bne CODE_85C0F7             ; $85C0EE
    lda.l $850000,x             ; $85C0F0
    sta.w $1E1E                 ; $85C0F4
CODE_85C0F7:
    ldx.b $18                   ; $85C0F7
    tya                         ; $85C0F9
    bit.w #$0002                ; $85C0FA
    beq CODE_85C106             ; $85C0FD
    lda.b $1C                   ; $85C0FF
    ora.w #$0001                ; $85C101
    bra CODE_85C108             ; $85C104
CODE_85C106:
    lda.b $1C                   ; $85C106
CODE_85C108:
    asl a                       ; $85C108
    bit.w #$0040                ; $85C109
    bne CODE_85C114             ; $85C10C
    adc.l $840006,x             ; $85C10E
    bra CODE_85C11B             ; $85C112
CODE_85C114:
    and.w #$003F                ; $85C114
    adc.l $840008,x             ; $85C117
CODE_85C11B:
    tax                         ; $85C11B
    lda.l $7E5391,x             ; $85C11C
    sta.w $3CEC,y               ; $85C120
    sta.w $3D2C,y               ; $85C123
    ldx.b $18                   ; $85C126
    lda.l $840004,x             ; $85C128
    tax                         ; $85C12C
    lda.l $850000,x             ; $85C12D
    tay                         ; $85C131
    lda.l $850002,x             ; $85C132
    cmp.b $1A                   ; $85C136
    bcs CODE_85C14F             ; $85C138
    incbin !baserom:$2C13A..$2C14F
CODE_85C14F:
    tya                         ; $85C14F
    ldy.b $1A                   ; $85C150
    sta.w $3BEC,y               ; $85C152
    sta.w $3C2C,y               ; $85C155
    inc.b $16                   ; $85C158
    lda.b $16                   ; $85C15A
    cmp.w $019A                 ; $85C15C
    bne CODE_85C165             ; $85C15F
    incbin !baserom:$2C161..$2C165
CODE_85C165:
    dec.b $14                   ; $85C165
    beq CODE_85C16C             ; $85C167
    jmp.w $C087                 ; $85C169
CODE_85C16C:
    pea.w $837E                 ; $85C16C
    plb                         ; $85C16F
    ldx.b $A0                   ; $85C170
    lda.w $0226                 ; $85C172
    cmp.l $7E5B91,x             ; $85C175
    db $B0,$03 ; bcs            ; $85C179
    jmp.w $C264                 ; $85C17B
    incbin !baserom:$2C17E..$2C264
    plb                         ; $85C264
    lda.w $0226                 ; $85C265
    cmp.w $019A                 ; $85C268
    bcc CODE_85C278             ; $85C26B
    incbin !baserom:$2C26D..$2C278
CODE_85C278:
    sta.w $0226                 ; $85C278
    sta.w $023E                 ; $85C27B
    stz.w $0260                 ; $85C27E
    ldx.b $9C                   ; $85C281
    lda.w $0226                 ; $85C283
    sec                         ; $85C286
    sbc.l $840000,x             ; $85C287
    bcc CODE_85C2A1             ; $85C28B
    incbin !baserom:$2C28D..$2C2A1
CODE_85C2A1:
    lda.w $0260                 ; $85C2A1
    and.w #$00FF                ; $85C2A4
    sta.w $0142                 ; $85C2A7
    lda.w $0226                 ; $85C2AA
    clc                         ; $85C2AD
    adc.w #$0001                ; $85C2AE
    cmp.w $019A                 ; $85C2B1
    bcc CODE_85C2B9             ; $85C2B4
    incbin !baserom:$2C2B6..$2C2B9
CODE_85C2B9:
    jsr.w $CD78                 ; $85C2B9
    sta.b $14                   ; $85C2BC
    lda.w $0226                 ; $85C2BE
    sec                         ; $85C2C1
    sbc.w #$0003                ; $85C2C2
    bcs CODE_85C2CA             ; $85C2C5
    incbin !baserom:$2C2C7..$2C2CA
CODE_85C2CA:
    jsr.w $CD78                 ; $85C2CA
    sta.b $16                   ; $85C2CD
    lda.w #$0000                ; $85C2CF
    sta.l $308000               ; $85C2D2
    lda.b $14                   ; $85C2D6
    sec                         ; $85C2D8
    sbc.b $16                   ; $85C2D9
    sta.l $308000               ; $85C2DB
    lda.w $0225                 ; $85C2DF
    clc                         ; $85C2E2
    adc.w #$0100                ; $85C2E3
    and.w #$03FF                ; $85C2E6
    sta.l $308000               ; $85C2E9
    lda.b $16                   ; $85C2ED
    cmp.w #$8000                ; $85C2EF
    rol a                       ; $85C2F2
    rol a                       ; $85C2F3
    xba                         ; $85C2F4
    and.w #$03FF                ; $85C2F5
    bit.w #$0080                ; $85C2F8
    beq CODE_85C300             ; $85C2FB
    incbin !baserom:$2C2FD..$2C300
CODE_85C300:
    sta.l $308000               ; $85C300
    clc                         ; $85C304
    adc.l $308000               ; $85C305
    clc                         ; $85C309
    adc.w #$0009                ; $85C30A
    eor.w #$FFFF                ; $85C30D
    inc a                       ; $85C310
    sta.w $0237                 ; $85C311
    plb                         ; $85C314
    rtl                         ; $85C315
    pea.w $837E                 ; $85C316
    plb                         ; $85C319
    lda.w $02F1                 ; $85C31A
    asl a                       ; $85C31D
    and.w #$003E                ; $85C31E
    tax                         ; $85C321
    lda.w $02D9                 ; $85C322
    sec                         ; $85C325
    sbc.w $02F1                 ; $85C326
    db $F0,$79 ; beq            ; $85C329
    tay                         ; $85C32B
    lda.w #$0000                ; $85C32C
    sta.l $308000               ; $85C32F
    sec                         ; $85C333
    sbc.w $02EF                 ; $85C334
    xba                         ; $85C337
    and.w #$00FF                ; $85C338
    sta.l $308000               ; $85C33B
    lda.w $406E,x               ; $85C33F
    sta.l $308000               ; $85C342
    nop                         ; $85C346
    lda.l $308000               ; $85C347
    xba                         ; $85C34B
    pha                         ; $85C34C
    and.w #$FF00                ; $85C34D
    sta.b $14                   ; $85C350
    pla                         ; $85C352
    and.w #$00FF                ; $85C353
    sta.b $16                   ; $85C356
    lda.l $308000               ; $85C358
    xba                         ; $85C35C
    and.w #$FF00                ; $85C35D
    ora.b $16                   ; $85C360
    sta.w $01D2                 ; $85C362
    inx                         ; $85C365
    inx                         ; $85C366
    dey                         ; $85C367
    beq CODE_85C375             ; $85C368
CODE_85C36A:
    adc.w $406E,x               ; $85C36A
    inx                         ; $85C36D
    inx                         ; $85C36E
    dey                         ; $85C36F
    bne CODE_85C36A             ; $85C370
    sta.w $01D2                 ; $85C372
CODE_85C375:
    lda.w #$0000                ; $85C375
    sta.l $308000               ; $85C378
    lda.w $02D7                 ; $85C37C
    and.w #$FF00                ; $85C37F
    lsr a                       ; $85C382
    sta.l $308000               ; $85C383
    lda.w $406E,x               ; $85C387
    asl a                       ; $85C38A
    sta.l $308000               ; $85C38B
    nop                         ; $85C38F
    lda.l $308000               ; $85C390
    clc                         ; $85C394
    adc.b $14                   ; $85C395
    lda.l $308000               ; $85C397
    clc                         ; $85C39B
    adc.w $01D2                 ; $85C39C
    sta.w $01D2                 ; $85C39F
    bra CODE_85C3CE             ; $85C3A2
    incbin !baserom:$2C3A4..$2C3CE
CODE_85C3CE:
    lda.w $02F1                 ; $85C3CE
    clc                         ; $85C3D1
    adc.w #$0020                ; $85C3D2
    cmp.w $019A                 ; $85C3D5
    bcc CODE_85C3DD             ; $85C3D8
    incbin !baserom:$2C3DA..$2C3DD
CODE_85C3DD:
    sta.b $16                   ; $85C3DD
    lda.w $02D7                 ; $85C3DF
    and.w #$FF00                ; $85C3E2
    sta.w $02EF                 ; $85C3E5
    lda.w $02D9                 ; $85C3E8
    sec                         ; $85C3EB
    sbc.w $02F1                 ; $85C3EC
    bne CODE_85C3F4             ; $85C3EF
    incbin !baserom:$2C3F1..$2C3F4
CODE_85C3F4:
    sta.b $14                   ; $85C3F4
    lda.w $02F7                 ; $85C3F6
    lsr a                       ; $85C3F9
    bcs CODE_85C405             ; $85C3FA
    incbin !baserom:$2C3FC..$2C405
CODE_85C405:
    lda.w $02F1                 ; $85C405
    asl a                       ; $85C408
    and.w #$003E                ; $85C409
    tax                         ; $85C40C
    lda.l $7E3E6E,x             ; $85C40D
    sec                         ; $85C411
    sbc.l $7E3E6C,x             ; $85C412
    sta.w $1F02                 ; $85C416
    lda.l $7E3DEE,x             ; $85C419
    sec                         ; $85C41D
    sbc.l $7E3DEC,x             ; $85C41E
    sta.w $1F04                 ; $85C422
    lda.w #$0000                ; $85C425
    sta.l $308000               ; $85C428
    lda.b $16                   ; $85C42C
    asl a                       ; $85C42E
    asl a                       ; $85C42F
    xba                         ; $85C430
    and.w #$00FF                ; $85C431
    sta.l $308000               ; $85C434
    lda.w #$000A                ; $85C438
    sta.l $308000               ; $85C43B
    nop                         ; $85C43F
    lda.l $308000               ; $85C440
    clc                         ; $85C444
    adc.b $98                   ; $85C445
    sta.b $18                   ; $85C447
    tax                         ; $85C449
    lda.l $308000               ; $85C44A
    lda.b $16                   ; $85C44E
    asl a                       ; $85C450
    and.w #$003E                ; $85C451
    tay                         ; $85C454
    sta.b $1A                   ; $85C455
    lda.b $16                   ; $85C457
    and.w #$003E                ; $85C459
    sta.b $1C                   ; $85C45C
    adc.l $840000,x             ; $85C45E
    tax                         ; $85C462
    lda.l $850000,x             ; $85C463
    sta.w $406C,y               ; $85C467
    sta.w $40AC,y               ; $85C46A
    ldx.b $18                   ; $85C46D
    lda.b $1C                   ; $85C46F
    clc                         ; $85C471
    adc.l $840002,x             ; $85C472
    tax                         ; $85C476
    lda.l $850000,x             ; $85C477
    sec                         ; $85C47B
    sbc.w $1E20                 ; $85C47C
    sta.w $40EC,y               ; $85C47F
    sta.w $412C,y               ; $85C482
    tya                         ; $85C485
    and.w #$0006                ; $85C486
    cmp.w #$0006                ; $85C489
    bne CODE_85C495             ; $85C48C
    lda.l $850000,x             ; $85C48E
    sta.w $1E20                 ; $85C492
CODE_85C495:
    ldx.b $18                   ; $85C495
    tya                         ; $85C497
    bit.w #$0002                ; $85C498
    beq CODE_85C4A4             ; $85C49B
    lda.b $1C                   ; $85C49D
    ora.w #$0001                ; $85C49F
    bra CODE_85C4A6             ; $85C4A2
CODE_85C4A4:
    lda.b $1C                   ; $85C4A4
CODE_85C4A6:
    asl a                       ; $85C4A6
    bit.w #$0040                ; $85C4A7
    bne CODE_85C4B2             ; $85C4AA
    adc.l $840006,x             ; $85C4AC
    bra CODE_85C4B9             ; $85C4B0
CODE_85C4B2:
    and.w #$003F                ; $85C4B2
    adc.l $840008,x             ; $85C4B5
CODE_85C4B9:
    tax                         ; $85C4B9
    lda.l $7E5391,x             ; $85C4BA
    sta.w $426C,y               ; $85C4BE
    sta.w $42AC,y               ; $85C4C1
    ldx.b $18                   ; $85C4C4
    lda.l $840004,x             ; $85C4C6
    tax                         ; $85C4CA
    lda.l $850000,x             ; $85C4CB
    tay                         ; $85C4CF
    lda.l $850002,x             ; $85C4D0
    cmp.b $1A                   ; $85C4D4
    bcs CODE_85C4ED             ; $85C4D6
    incbin !baserom:$2C4D8..$2C4ED
CODE_85C4ED:
    tya                         ; $85C4ED
    ldy.b $1A                   ; $85C4EE
    sta.w $416C,y               ; $85C4F0
    sta.w $41AC,y               ; $85C4F3
    inc.b $16                   ; $85C4F6
    lda.b $16                   ; $85C4F8
    cmp.w $019A                 ; $85C4FA
    bne CODE_85C503             ; $85C4FD
    incbin !baserom:$2C4FF..$2C503
CODE_85C503:
    dec.b $14                   ; $85C503
    beq CODE_85C50A             ; $85C505
    jmp.w $C425                 ; $85C507
CODE_85C50A:
    pea.w $837E                 ; $85C50A
    plb                         ; $85C50D
    ldx.b $A4                   ; $85C50E
    lda.w $02D9                 ; $85C510
    cmp.l $7E5B91,x             ; $85C513
    db $B0,$03 ; bcs            ; $85C517
    jmp.w $C610                 ; $85C519
    incbin !baserom:$2C51C..$2C610
    plb                         ; $85C610
    lda.w $02D9                 ; $85C611
    cmp.w $019A                 ; $85C614
    bcc CODE_85C624             ; $85C617
    incbin !baserom:$2C619..$2C624
CODE_85C624:
    sta.w $02D9                 ; $85C624
    sta.w $02F1                 ; $85C627
    stz.w $0313                 ; $85C62A
    ldx.b $9C                   ; $85C62D
    lda.w $02D9                 ; $85C62F
    sec                         ; $85C632
    sbc.l $840000,x             ; $85C633
    bcc CODE_85C64D             ; $85C637
    incbin !baserom:$2C639..$2C64D
CODE_85C64D:
    lda.w $0313                 ; $85C64D
    and.w #$00FF                ; $85C650
    sta.w $0144                 ; $85C653
    lda.w $02D9                 ; $85C656
    clc                         ; $85C659
    adc.w #$0001                ; $85C65A
    cmp.w $019A                 ; $85C65D
    bcc CODE_85C665             ; $85C660
    incbin !baserom:$2C662..$2C665
CODE_85C665:
    jsr.w $CD78                 ; $85C665
    sta.b $14                   ; $85C668
    lda.w $02D9                 ; $85C66A
    sec                         ; $85C66D
    sbc.w #$0003                ; $85C66E
    bcs CODE_85C676             ; $85C671
    incbin !baserom:$2C673..$2C676
CODE_85C676:
    jsr.w $CD78                 ; $85C676
    sta.b $16                   ; $85C679
    lda.w #$0000                ; $85C67B
    sta.l $308000               ; $85C67E
    lda.b $14                   ; $85C682
    sec                         ; $85C684
    sbc.b $16                   ; $85C685
    sta.l $308000               ; $85C687
    lda.w $02D8                 ; $85C68B
    clc                         ; $85C68E
    adc.w #$0100                ; $85C68F
    and.w #$03FF                ; $85C692
    sta.l $308000               ; $85C695
    lda.b $16                   ; $85C699
    cmp.w #$8000                ; $85C69B
    rol a                       ; $85C69E
    rol a                       ; $85C69F
    xba                         ; $85C6A0
    and.w #$03FF                ; $85C6A1
    bit.w #$0080                ; $85C6A4
    beq CODE_85C6AC             ; $85C6A7
    incbin !baserom:$2C6A9..$2C6AC
CODE_85C6AC:
    sta.l $308000               ; $85C6AC
    clc                         ; $85C6B0
    adc.l $308000               ; $85C6B1
    clc                         ; $85C6B5
    adc.w #$0009                ; $85C6B6
    eor.w #$FFFF                ; $85C6B9
    inc a                       ; $85C6BC
    sta.w $02EA                 ; $85C6BD
    plb                         ; $85C6C0
    rtl                         ; $85C6C1
    pea.w $837E                 ; $85C6C2
    plb                         ; $85C6C5
    lda.w $02A0                 ; $85C6C6
    asl a                       ; $85C6C9
    and.w #$003E                ; $85C6CA
    tax                         ; $85C6CD
    lda.w $0288                 ; $85C6CE
    sec                         ; $85C6D1
    sbc.w $02A0                 ; $85C6D2
    db $F0,$79 ; beq            ; $85C6D5
    tay                         ; $85C6D7
    lda.w #$0000                ; $85C6D8
    sta.l $308000               ; $85C6DB
    sec                         ; $85C6DF
    sbc.w $029E                 ; $85C6E0
    xba                         ; $85C6E3
    and.w #$00FF                ; $85C6E4
    sta.l $308000               ; $85C6E7
    lda.w $3DEE,x               ; $85C6EB
    sta.l $308000               ; $85C6EE
    nop                         ; $85C6F2
    lda.l $308000               ; $85C6F3
    xba                         ; $85C6F7
    pha                         ; $85C6F8
    and.w #$FF00                ; $85C6F9
    sta.b $14                   ; $85C6FC
    pla                         ; $85C6FE
    and.w #$00FF                ; $85C6FF
    sta.b $16                   ; $85C702
    lda.l $308000               ; $85C704
    xba                         ; $85C708
    and.w #$FF00                ; $85C709
    ora.b $16                   ; $85C70C
    sta.w $01D6                 ; $85C70E
    inx                         ; $85C711
    inx                         ; $85C712
    dey                         ; $85C713
    beq CODE_85C721             ; $85C714
CODE_85C716:
    adc.w $3DEE,x               ; $85C716
    inx                         ; $85C719
    inx                         ; $85C71A
    dey                         ; $85C71B
    bne CODE_85C716             ; $85C71C
    sta.w $01D6                 ; $85C71E
CODE_85C721:
    lda.w #$0000                ; $85C721
    sta.l $308000               ; $85C724
    lda.w $0286                 ; $85C728
    and.w #$FF00                ; $85C72B
    lsr a                       ; $85C72E
    sta.l $308000               ; $85C72F
    lda.w $3DEE,x               ; $85C733
    asl a                       ; $85C736
    sta.l $308000               ; $85C737
    nop                         ; $85C73B
    lda.l $308000               ; $85C73C
    clc                         ; $85C740
    adc.b $14                   ; $85C741
    lda.l $308000               ; $85C743
    clc                         ; $85C747
    adc.w $01D6                 ; $85C748
    sta.w $01D6                 ; $85C74B
    bra CODE_85C77A             ; $85C74E
    incbin !baserom:$2C750..$2C77A
CODE_85C77A:
    lda.w $02A0                 ; $85C77A
    clc                         ; $85C77D
    adc.w #$0020                ; $85C77E
    cmp.w $019A                 ; $85C781
    bcc CODE_85C789             ; $85C784
    incbin !baserom:$2C786..$2C789
CODE_85C789:
    sta.b $16                   ; $85C789
    lda.w $0286                 ; $85C78B
    and.w #$FF00                ; $85C78E
    sta.w $029E                 ; $85C791
    lda.w $0288                 ; $85C794
    sec                         ; $85C797
    sbc.w $02A0                 ; $85C798
    bne CODE_85C7A0             ; $85C79B
    incbin !baserom:$2C79D..$2C7A0
CODE_85C7A0:
    sta.b $14                   ; $85C7A0
    lda.w $02A6                 ; $85C7A2
    lsr a                       ; $85C7A5
    bcs CODE_85C7B1             ; $85C7A6
    incbin !baserom:$2C7A8..$2C7B1
CODE_85C7B1:
    lda.w #$0000                ; $85C7B1
    sta.l $308000               ; $85C7B4
    lda.b $16                   ; $85C7B8
    asl a                       ; $85C7BA
    asl a                       ; $85C7BB
    xba                         ; $85C7BC
    and.w #$00FF                ; $85C7BD
    sta.l $308000               ; $85C7C0
    lda.w #$000A                ; $85C7C4
    sta.l $308000               ; $85C7C7
    nop                         ; $85C7CB
    lda.l $308000               ; $85C7CC
    clc                         ; $85C7D0
    adc.b $98                   ; $85C7D1
    sta.b $18                   ; $85C7D3
    tax                         ; $85C7D5
    lda.l $308000               ; $85C7D6
    lda.b $16                   ; $85C7DA
    asl a                       ; $85C7DC
    and.w #$003E                ; $85C7DD
    tay                         ; $85C7E0
    sta.b $1A                   ; $85C7E1
    lda.b $16                   ; $85C7E3
    and.w #$003E                ; $85C7E5
    sta.b $1C                   ; $85C7E8
    adc.l $840000,x             ; $85C7EA
    tax                         ; $85C7EE
    lda.l $850000,x             ; $85C7EF
    sta.w $3DEC,y               ; $85C7F3
    sta.w $3E2C,y               ; $85C7F6
    ldx.b $18                   ; $85C7F9
    lda.b $1C                   ; $85C7FB
    clc                         ; $85C7FD
    adc.l $840002,x             ; $85C7FE
    tax                         ; $85C802
    lda.l $850000,x             ; $85C803
    sec                         ; $85C807
    sbc.w $1E22                 ; $85C808
    sta.w $3E6C,y               ; $85C80B
    sta.w $3EAC,y               ; $85C80E
    tya                         ; $85C811
    and.w #$0006                ; $85C812
    cmp.w #$0006                ; $85C815
    bne CODE_85C821             ; $85C818
    lda.l $850000,x             ; $85C81A
    sta.w $1E22                 ; $85C81E
CODE_85C821:
    ldx.b $18                   ; $85C821
    tya                         ; $85C823
    bit.w #$0002                ; $85C824
    beq CODE_85C830             ; $85C827
    lda.b $1C                   ; $85C829
    ora.w #$0001                ; $85C82B
    bra CODE_85C832             ; $85C82E
CODE_85C830:
    lda.b $1C                   ; $85C830
CODE_85C832:
    asl a                       ; $85C832
    bit.w #$0040                ; $85C833
    bne CODE_85C83E             ; $85C836
    adc.l $840006,x             ; $85C838
    bra CODE_85C845             ; $85C83C
CODE_85C83E:
    and.w #$003F                ; $85C83E
    adc.l $840008,x             ; $85C841
CODE_85C845:
    tax                         ; $85C845
    lda.l $7E5391,x             ; $85C846
    sta.w $3FEC,y               ; $85C84A
    sta.w $402C,y               ; $85C84D
    ldx.b $18                   ; $85C850
    lda.l $840004,x             ; $85C852
    tax                         ; $85C856
    lda.l $850000,x             ; $85C857
    tay                         ; $85C85B
    lda.l $850002,x             ; $85C85C
    cmp.b $1A                   ; $85C860
    bcs CODE_85C879             ; $85C862
    incbin !baserom:$2C864..$2C879
CODE_85C879:
    tya                         ; $85C879
    ldy.b $1A                   ; $85C87A
    sta.w $3EEC,y               ; $85C87C
    sta.w $3F2C,y               ; $85C87F
    inc.b $16                   ; $85C882
    lda.b $16                   ; $85C884
    cmp.w $019A                 ; $85C886
    bne CODE_85C88D             ; $85C889
    incbin !baserom:$2C88B..$2C88D
CODE_85C88D:
    dec.b $14                   ; $85C88D
    beq CODE_85C894             ; $85C88F
    jmp.w $C7B1                 ; $85C891
CODE_85C894:
    pea.w $837E                 ; $85C894
    plb                         ; $85C897
    ldx.b $A8                   ; $85C898
    lda.w $0288                 ; $85C89A
    cmp.l $7E5B91,x             ; $85C89D
    db $B0,$03 ; bcs            ; $85C8A1
    jmp.w $C992                 ; $85C8A3
    incbin !baserom:$2C8A6..$2C992
    plb                         ; $85C992
    lda.w $0288                 ; $85C993
    cmp.w $019A                 ; $85C996
    bcc CODE_85C9A6             ; $85C999
    incbin !baserom:$2C99B..$2C9A6
CODE_85C9A6:
    sta.w $0288                 ; $85C9A6
    sta.w $02A0                 ; $85C9A9
    lda.w $0288                 ; $85C9AC
    clc                         ; $85C9AF
    adc.w #$0001                ; $85C9B0
    cmp.w $019A                 ; $85C9B3
    bcc CODE_85C9BB             ; $85C9B6
    incbin !baserom:$2C9B8..$2C9BB
CODE_85C9BB:
    jsr.w $CD78                 ; $85C9BB
    sta.b $14                   ; $85C9BE
    lda.w $0288                 ; $85C9C0
    sec                         ; $85C9C3
    sbc.w #$0003                ; $85C9C4
    bcs CODE_85C9CC             ; $85C9C7
    incbin !baserom:$2C9C9..$2C9CC
CODE_85C9CC:
    jsr.w $CD78                 ; $85C9CC
    sta.b $16                   ; $85C9CF
    lda.w #$0000                ; $85C9D1
    sta.l $308000               ; $85C9D4
    lda.b $14                   ; $85C9D8
    sec                         ; $85C9DA
    sbc.b $16                   ; $85C9DB
    sta.l $308000               ; $85C9DD
    lda.w $0287                 ; $85C9E1
    clc                         ; $85C9E4
    adc.w #$0100                ; $85C9E5
    and.w #$03FF                ; $85C9E8
    sta.l $308000               ; $85C9EB
    lda.b $16                   ; $85C9EF
    cmp.w #$8000                ; $85C9F1
    rol a                       ; $85C9F4
    rol a                       ; $85C9F5
    xba                         ; $85C9F6
    and.w #$03FF                ; $85C9F7
    bit.w #$0080                ; $85C9FA
    beq CODE_85CA02             ; $85C9FD
    incbin !baserom:$2C9FF..$2CA02
CODE_85CA02:
    sta.l $308000               ; $85CA02
    clc                         ; $85CA06
    adc.l $308000               ; $85CA07
    clc                         ; $85CA0B
    adc.w #$0009                ; $85CA0C
    eor.w #$FFFF                ; $85CA0F
    inc a                       ; $85CA12
    sta.w $0299                 ; $85CA13
    plb                         ; $85CA16
    rtl                         ; $85CA17
    pea.w $837E                 ; $85CA18
    plb                         ; $85CA1B
    lda.w $0353                 ; $85CA1C
    asl a                       ; $85CA1F
    and.w #$003E                ; $85CA20
    tax                         ; $85CA23
    lda.w $033B                 ; $85CA24
    sec                         ; $85CA27
    sbc.w $0353                 ; $85CA28
    db $F0,$79 ; beq            ; $85CA2B
    tay                         ; $85CA2D
    lda.w #$0000                ; $85CA2E
    sta.l $308000               ; $85CA31
    sec                         ; $85CA35
    sbc.w $0351                 ; $85CA36
    xba                         ; $85CA39
    and.w #$00FF                ; $85CA3A
    sta.l $308000               ; $85CA3D
    lda.w $436E,x               ; $85CA41
    sta.l $308000               ; $85CA44
    nop                         ; $85CA48
    lda.l $308000               ; $85CA49
    xba                         ; $85CA4D
    pha                         ; $85CA4E
    and.w #$FF00                ; $85CA4F
    sta.b $14                   ; $85CA52
    pla                         ; $85CA54
    and.w #$00FF                ; $85CA55
    sta.b $16                   ; $85CA58
    lda.l $308000               ; $85CA5A
    xba                         ; $85CA5E
    and.w #$FF00                ; $85CA5F
    ora.b $16                   ; $85CA62
    sta.w $01DA                 ; $85CA64
    inx                         ; $85CA67
    inx                         ; $85CA68
    dey                         ; $85CA69
    beq CODE_85CA77             ; $85CA6A
CODE_85CA6C:
    adc.w $436E,x               ; $85CA6C
    inx                         ; $85CA6F
    inx                         ; $85CA70
    dey                         ; $85CA71
    bne CODE_85CA6C             ; $85CA72
    sta.w $01DA                 ; $85CA74
CODE_85CA77:
    lda.w #$0000                ; $85CA77
    sta.l $308000               ; $85CA7A
    lda.w $0339                 ; $85CA7E
    and.w #$FF00                ; $85CA81
    lsr a                       ; $85CA84
    sta.l $308000               ; $85CA85
    lda.w $436E,x               ; $85CA89
    asl a                       ; $85CA8C
    sta.l $308000               ; $85CA8D
    nop                         ; $85CA91
    lda.l $308000               ; $85CA92
    clc                         ; $85CA96
    adc.b $14                   ; $85CA97
    lda.l $308000               ; $85CA99
    clc                         ; $85CA9D
    adc.w $01DA                 ; $85CA9E
    sta.w $01DA                 ; $85CAA1
    bra CODE_85CAD0             ; $85CAA4
    incbin !baserom:$2CAA6..$2CAD0
CODE_85CAD0:
    lda.w $0353                 ; $85CAD0
    clc                         ; $85CAD3
    adc.w #$0020                ; $85CAD4
    cmp.w $019A                 ; $85CAD7
    bcc CODE_85CADF             ; $85CADA
    incbin !baserom:$2CADC..$2CADF
CODE_85CADF:
    sta.b $16                   ; $85CADF
    lda.w $0339                 ; $85CAE1
    and.w #$FF00                ; $85CAE4
    sta.w $0351                 ; $85CAE7
    lda.w $033B                 ; $85CAEA
    sec                         ; $85CAED
    sbc.w $0353                 ; $85CAEE
    bne CODE_85CAF6             ; $85CAF1
    incbin !baserom:$2CAF3..$2CAF6
CODE_85CAF6:
    sta.b $14                   ; $85CAF6
    lda.w $0359                 ; $85CAF8
    lsr a                       ; $85CAFB
    bcs CODE_85CB07             ; $85CAFC
    incbin !baserom:$2CAFE..$2CB07
CODE_85CB07:
    lda.w #$0000                ; $85CB07
    sta.l $308000               ; $85CB0A
    lda.b $16                   ; $85CB0E
    asl a                       ; $85CB10
    asl a                       ; $85CB11
    xba                         ; $85CB12
    and.w #$00FF                ; $85CB13
    sta.l $308000               ; $85CB16
    lda.w #$000A                ; $85CB1A
    sta.l $308000               ; $85CB1D
    nop                         ; $85CB21
    lda.l $308000               ; $85CB22
    clc                         ; $85CB26
    adc.b $98                   ; $85CB27
    sta.b $18                   ; $85CB29
    tax                         ; $85CB2B
    lda.l $308000               ; $85CB2C
    lda.b $16                   ; $85CB30
    asl a                       ; $85CB32
    and.w #$003E                ; $85CB33
    tay                         ; $85CB36
    sta.b $1A                   ; $85CB37
    lda.b $16                   ; $85CB39
    and.w #$003E                ; $85CB3B
    sta.b $1C                   ; $85CB3E
    adc.l $840000,x             ; $85CB40
    tax                         ; $85CB44
    lda.l $850000,x             ; $85CB45
    sta.w $436C,y               ; $85CB49
    sta.w $43AC,y               ; $85CB4C
    ldx.b $18                   ; $85CB4F
    lda.b $1C                   ; $85CB51
    clc                         ; $85CB53
    adc.l $840002,x             ; $85CB54
    tax                         ; $85CB58
    lda.l $850000,x             ; $85CB59
    sec                         ; $85CB5D
    sbc.w $1E24                 ; $85CB5E
    sta.w $43EC,y               ; $85CB61
    sta.w $442C,y               ; $85CB64
    tya                         ; $85CB67
    and.w #$0006                ; $85CB68
    cmp.w #$0006                ; $85CB6B
    bne CODE_85CB77             ; $85CB6E
    lda.l $850000,x             ; $85CB70
    sta.w $1E24                 ; $85CB74
CODE_85CB77:
    ldx.b $18                   ; $85CB77
    tya                         ; $85CB79
    bit.w #$0002                ; $85CB7A
    beq CODE_85CB86             ; $85CB7D
    lda.b $1C                   ; $85CB7F
    ora.w #$0001                ; $85CB81
    bra CODE_85CB88             ; $85CB84
CODE_85CB86:
    lda.b $1C                   ; $85CB86
CODE_85CB88:
    asl a                       ; $85CB88
    bit.w #$0040                ; $85CB89
    bne CODE_85CB94             ; $85CB8C
    adc.l $840006,x             ; $85CB8E
    bra CODE_85CB9B             ; $85CB92
CODE_85CB94:
    and.w #$003F                ; $85CB94
    adc.l $840008,x             ; $85CB97
CODE_85CB9B:
    tax                         ; $85CB9B
    lda.l $7E5391,x             ; $85CB9C
    sta.w $456C,y               ; $85CBA0
    sta.w $45AC,y               ; $85CBA3
    ldx.b $18                   ; $85CBA6
    lda.l $840004,x             ; $85CBA8
    tax                         ; $85CBAC
    lda.l $850000,x             ; $85CBAD
    tay                         ; $85CBB1
    lda.l $850002,x             ; $85CBB2
    cmp.b $1A                   ; $85CBB6
    bcs CODE_85CBCF             ; $85CBB8
    incbin !baserom:$2CBBA..$2CBCF
CODE_85CBCF:
    tya                         ; $85CBCF
    ldy.b $1A                   ; $85CBD0
    sta.w $446C,y               ; $85CBD2
    sta.w $44AC,y               ; $85CBD5
    inc.b $16                   ; $85CBD8
    lda.b $16                   ; $85CBDA
    cmp.w $019A                 ; $85CBDC
    bne CODE_85CBE5             ; $85CBDF
    incbin !baserom:$2CBE1..$2CBE5
CODE_85CBE5:
    dec.b $14                   ; $85CBE5
    beq CODE_85CBEC             ; $85CBE7
    jmp.w $CB07                 ; $85CBE9
CODE_85CBEC:
    pea.w $837E                 ; $85CBEC
    plb                         ; $85CBEF
    ldx.b $AC                   ; $85CBF0
    lda.w $033B                 ; $85CBF2
    cmp.l $7E5B91,x             ; $85CBF5
    db $B0,$03 ; bcs            ; $85CBF9
    jmp.w $CCF2                 ; $85CBFB
    incbin !baserom:$2CBFE..$2CCF2
    plb                         ; $85CCF2
    lda.w $033B                 ; $85CCF3
    cmp.w $019A                 ; $85CCF6
    bcc CODE_85CD06             ; $85CCF9
    incbin !baserom:$2CCFB..$2CD06
CODE_85CD06:
    sta.w $033B                 ; $85CD06
    sta.w $0353                 ; $85CD09
    lda.w $033B                 ; $85CD0C
    clc                         ; $85CD0F
    adc.w #$0001                ; $85CD10
    cmp.w $019A                 ; $85CD13
    bcc CODE_85CD1B             ; $85CD16
    incbin !baserom:$2CD18..$2CD1B
CODE_85CD1B:
    jsr.w $CD78                 ; $85CD1B
    sta.b $14                   ; $85CD1E
    lda.w $033B                 ; $85CD20
    sec                         ; $85CD23
    sbc.w #$0003                ; $85CD24
    bcs CODE_85CD2C             ; $85CD27
    incbin !baserom:$2CD29..$2CD2C
CODE_85CD2C:
    jsr.w $CD78                 ; $85CD2C
    sta.b $16                   ; $85CD2F
    lda.w #$0000                ; $85CD31
    sta.l $308000               ; $85CD34
    lda.b $14                   ; $85CD38
    sec                         ; $85CD3A
    sbc.b $16                   ; $85CD3B
    sta.l $308000               ; $85CD3D
    lda.w $033A                 ; $85CD41
    clc                         ; $85CD44
    adc.w #$0100                ; $85CD45
    and.w #$03FF                ; $85CD48
    sta.l $308000               ; $85CD4B
    lda.b $16                   ; $85CD4F
    cmp.w #$8000                ; $85CD51
    rol a                       ; $85CD54
    rol a                       ; $85CD55
    xba                         ; $85CD56
    and.w #$03FF                ; $85CD57
    bit.w #$0080                ; $85CD5A
    beq CODE_85CD62             ; $85CD5D
    incbin !baserom:$2CD5F..$2CD62
CODE_85CD62:
    sta.l $308000               ; $85CD62
    clc                         ; $85CD66
    adc.l $308000               ; $85CD67
    clc                         ; $85CD6B
    adc.w #$0009                ; $85CD6C
    eor.w #$FFFF                ; $85CD6F
    inc a                       ; $85CD72
    sta.w $034C                 ; $85CD73
    plb                         ; $85CD76
    rtl                         ; $85CD77
    tay                         ; $85CD78
    lda.w #$0000                ; $85CD79
    sta.l $308000               ; $85CD7C
    tya                         ; $85CD80
    asl a                       ; $85CD81
    asl a                       ; $85CD82
    xba                         ; $85CD83
    and.w #$00FF                ; $85CD84
    sta.l $308000               ; $85CD87
    lda.w #$000A                ; $85CD8B
    sta.l $308000               ; $85CD8E
    nop                         ; $85CD92
    lda.l $308000               ; $85CD93
    clc                         ; $85CD97
    adc.b $98                   ; $85CD98
    sta.b $18                   ; $85CD9A
    tax                         ; $85CD9C
    lda.l $308000               ; $85CD9D
    tya                         ; $85CDA1
    and.w #$003E                ; $85CDA2
    adc.l $840002,x             ; $85CDA5
    tax                         ; $85CDA9
    lda.l $850000,x             ; $85CDAA
    rts                         ; $85CDAE
    sty.b $14                   ; $85CDAF
    lda.w $F533,y               ; $85CDB1
    sta.b $16                   ; $85CDB4
    lda.w $F733,y               ; $85CDB6
    sta.b $18                   ; $85CDB9
    ldy.w $00FE                 ; $85CDBB
    cpy.b $14                   ; $85CDBE
    beq CODE_85CDE4             ; $85CDC0
CODE_85CDC2:
    lda.b $18                   ; $85CDC2
    cmp.w $F733,y               ; $85CDC4
    bne CODE_85CDD9             ; $85CDC7
    lda.b $16                   ; $85CDC9
    cmp.w $F533,y               ; $85CDCB
    bne CODE_85CDD9             ; $85CDCE
    incbin !baserom:$2CDD0..$2CDD9
CODE_85CDD9:
    iny                         ; $85CDD9
    iny                         ; $85CDDA
    tya                         ; $85CDDB
    and.w #$00FF                ; $85CDDC
    tay                         ; $85CDDF
    cpy.b $14                   ; $85CDE0
    bne CODE_85CDC2             ; $85CDE2
CODE_85CDE4:
    rtl                         ; $85CDE4
    incbin !baserom:$2CDE5..$30000
