; bank $81 (file offset $008000)
org $818000

    incbin !baserom:$8000..$80D0
    jsr.w $80F6                 ; $8180D0
    lda.b $74                   ; $8180D3
    sta.b $72                   ; $8180D5
    stz.b $74                   ; $8180D7
    rtl                         ; $8180D9
    jsr.w $8101                 ; $8180DA
    lda.b $70                   ; $8180DD
    sta.b $72                   ; $8180DF
    lda.b $74                   ; $8180E1
    sta.b $70                   ; $8180E3
    lda.b $6E                   ; $8180E5
    sec                         ; $8180E7
    sbc.w #$0900                ; $8180E8
    sta.b $74                   ; $8180EB
    ldx.b $6C                   ; $8180ED
    lda.b $6E                   ; $8180EF
    sta.b $6C                   ; $8180F1
    stx.b $6E                   ; $8180F3
    rtl                         ; $8180F5
    ldx.w #$0900                ; $8180F6
    phd                         ; $8180F9
    lda.w #$0B00                ; $8180FA
    tcd                         ; $8180FD
    jmp.w $810A                 ; $8180FE
    phd                         ; $818101
    lda.b $6C                   ; $818102
    tax                         ; $818104
    clc                         ; $818105
    adc.w #$0200                ; $818106
    tcd                         ; $818109
    sep.b #$20                  ; $81810A
    asl.b $01                   ; $81810C
    ror a                       ; $81810E
    lsr.b $02                   ; $81810F
    ror a                       ; $818111
    asl.b $05                   ; $818112
    ror a                       ; $818114
    lsr.b $06                   ; $818115
    ror a                       ; $818117
    asl.b $09                   ; $818118
    ror a                       ; $81811A
    lsr.b $0A                   ; $81811B
    ror a                       ; $81811D
    asl.b $0D                   ; $81811E
    ror a                       ; $818120
    lsr.b $0E                   ; $818121
    ror a                       ; $818123
    sta.b $00                   ; $818124
    asl.b $11                   ; $818126
    ror a                       ; $818128
    lsr.b $12                   ; $818129
    ror a                       ; $81812B
    asl.b $15                   ; $81812C
    ror a                       ; $81812E
    lsr.b $16                   ; $81812F
    ror a                       ; $818131
    asl.b $19                   ; $818132
    ror a                       ; $818134
    lsr.b $1A                   ; $818135
    ror a                       ; $818137
    asl.b $1D                   ; $818138
    ror a                       ; $81813A
    lsr.b $1E                   ; $81813B
    ror a                       ; $81813D
    sta.b $01                   ; $81813E
    asl.b $21                   ; $818140
    ror a                       ; $818142
    lsr.b $22                   ; $818143
    ror a                       ; $818145
    asl.b $25                   ; $818146
    ror a                       ; $818148
    lsr.b $26                   ; $818149
    ror a                       ; $81814B
    asl.b $29                   ; $81814C
    ror a                       ; $81814E
    lsr.b $2A                   ; $81814F
    ror a                       ; $818151
    asl.b $2D                   ; $818152
    ror a                       ; $818154
    lsr.b $2E                   ; $818155
    ror a                       ; $818157
    sta.b $02                   ; $818158
    asl.b $31                   ; $81815A
    ror a                       ; $81815C
    lsr.b $32                   ; $81815D
    ror a                       ; $81815F
    asl.b $35                   ; $818160
    ror a                       ; $818162
    lsr.b $36                   ; $818163
    ror a                       ; $818165
    asl.b $39                   ; $818166
    ror a                       ; $818168
    lsr.b $3A                   ; $818169
    ror a                       ; $81816B
    asl.b $3D                   ; $81816C
    ror a                       ; $81816E
    lsr.b $3E                   ; $81816F
    ror a                       ; $818171
    sta.b $03                   ; $818172
    asl.b $41                   ; $818174
    ror a                       ; $818176
    lsr.b $42                   ; $818177
    ror a                       ; $818179
    asl.b $45                   ; $81817A
    ror a                       ; $81817C
    lsr.b $46                   ; $81817D
    ror a                       ; $81817F
    asl.b $49                   ; $818180
    ror a                       ; $818182
    lsr.b $4A                   ; $818183
    ror a                       ; $818185
    asl.b $4D                   ; $818186
    ror a                       ; $818188
    lsr.b $4E                   ; $818189
    ror a                       ; $81818B
    sta.b $04                   ; $81818C
    asl.b $51                   ; $81818E
    ror a                       ; $818190
    lsr.b $52                   ; $818191
    ror a                       ; $818193
    asl.b $55                   ; $818194
    ror a                       ; $818196
    lsr.b $56                   ; $818197
    ror a                       ; $818199
    asl.b $59                   ; $81819A
    ror a                       ; $81819C
    lsr.b $5A                   ; $81819D
    ror a                       ; $81819F
    asl.b $5D                   ; $8181A0
    ror a                       ; $8181A2
    lsr.b $5E                   ; $8181A3
    ror a                       ; $8181A5
    sta.b $05                   ; $8181A6
    asl.b $61                   ; $8181A8
    ror a                       ; $8181AA
    lsr.b $62                   ; $8181AB
    ror a                       ; $8181AD
    asl.b $65                   ; $8181AE
    ror a                       ; $8181B0
    lsr.b $66                   ; $8181B1
    ror a                       ; $8181B3
    asl.b $69                   ; $8181B4
    ror a                       ; $8181B6
    lsr.b $6A                   ; $8181B7
    ror a                       ; $8181B9
    asl.b $6D                   ; $8181BA
    ror a                       ; $8181BC
    lsr.b $6E                   ; $8181BD
    ror a                       ; $8181BF
    sta.b $06                   ; $8181C0
    asl.b $71                   ; $8181C2
    ror a                       ; $8181C4
    lsr.b $72                   ; $8181C5
    ror a                       ; $8181C7
    asl.b $75                   ; $8181C8
    ror a                       ; $8181CA
    lsr.b $76                   ; $8181CB
    ror a                       ; $8181CD
    asl.b $79                   ; $8181CE
    ror a                       ; $8181D0
    lsr.b $7A                   ; $8181D1
    ror a                       ; $8181D3
    asl.b $7D                   ; $8181D4
    ror a                       ; $8181D6
    lsr.b $7E                   ; $8181D7
    ror a                       ; $8181D9
    sta.b $07                   ; $8181DA
    asl.b $81                   ; $8181DC
    ror a                       ; $8181DE
    lsr.b $82                   ; $8181DF
    ror a                       ; $8181E1
    asl.b $85                   ; $8181E2
    ror a                       ; $8181E4
    lsr.b $86                   ; $8181E5
    ror a                       ; $8181E7
    asl.b $89                   ; $8181E8
    ror a                       ; $8181EA
    lsr.b $8A                   ; $8181EB
    ror a                       ; $8181ED
    asl.b $8D                   ; $8181EE
    ror a                       ; $8181F0
    lsr.b $8E                   ; $8181F1
    ror a                       ; $8181F3
    sta.b $08                   ; $8181F4
    asl.b $91                   ; $8181F6
    ror a                       ; $8181F8
    lsr.b $92                   ; $8181F9
    ror a                       ; $8181FB
    asl.b $95                   ; $8181FC
    ror a                       ; $8181FE
    lsr.b $96                   ; $8181FF
    ror a                       ; $818201
    asl.b $99                   ; $818202
    ror a                       ; $818204
    lsr.b $9A                   ; $818205
    ror a                       ; $818207
    asl.b $9D                   ; $818208
    ror a                       ; $81820A
    lsr.b $9E                   ; $81820B
    ror a                       ; $81820D
    sta.b $09                   ; $81820E
    asl.b $A1                   ; $818210
    ror a                       ; $818212
    lsr.b $A2                   ; $818213
    ror a                       ; $818215
    asl.b $A5                   ; $818216
    ror a                       ; $818218
    lsr.b $A6                   ; $818219
    ror a                       ; $81821B
    asl.b $A9                   ; $81821C
    ror a                       ; $81821E
    lsr.b $AA                   ; $81821F
    ror a                       ; $818221
    asl.b $AD                   ; $818222
    ror a                       ; $818224
    lsr.b $AE                   ; $818225
    ror a                       ; $818227
    sta.b $0A                   ; $818228
    asl.b $B1                   ; $81822A
    ror a                       ; $81822C
    lsr.b $B2                   ; $81822D
    ror a                       ; $81822F
    asl.b $B5                   ; $818230
    ror a                       ; $818232
    lsr.b $B6                   ; $818233
    ror a                       ; $818235
    asl.b $B9                   ; $818236
    ror a                       ; $818238
    lsr.b $BA                   ; $818239
    ror a                       ; $81823B
    asl.b $BD                   ; $81823C
    ror a                       ; $81823E
    lsr.b $BE                   ; $81823F
    ror a                       ; $818241
    sta.b $0B                   ; $818242
    asl.b $C1                   ; $818244
    ror a                       ; $818246
    lsr.b $C2                   ; $818247
    ror a                       ; $818249
    asl.b $C5                   ; $81824A
    ror a                       ; $81824C
    lsr.b $C6                   ; $81824D
    ror a                       ; $81824F
    asl.b $C9                   ; $818250
    ror a                       ; $818252
    lsr.b $CA                   ; $818253
    ror a                       ; $818255
    asl.b $CD                   ; $818256
    ror a                       ; $818258
    lsr.b $CE                   ; $818259
    ror a                       ; $81825B
    sta.b $0C                   ; $81825C
    asl.b $D1                   ; $81825E
    ror a                       ; $818260
    lsr.b $D2                   ; $818261
    ror a                       ; $818263
    asl.b $D5                   ; $818264
    ror a                       ; $818266
    lsr.b $D6                   ; $818267
    ror a                       ; $818269
    asl.b $D9                   ; $81826A
    ror a                       ; $81826C
    lsr.b $DA                   ; $81826D
    ror a                       ; $81826F
    asl.b $DD                   ; $818270
    ror a                       ; $818272
    lsr.b $DE                   ; $818273
    ror a                       ; $818275
    sta.b $0D                   ; $818276
    asl.b $E1                   ; $818278
    ror a                       ; $81827A
    lsr.b $E2                   ; $81827B
    ror a                       ; $81827D
    asl.b $E5                   ; $81827E
    ror a                       ; $818280
    lsr.b $E6                   ; $818281
    ror a                       ; $818283
    asl.b $E9                   ; $818284
    ror a                       ; $818286
    lsr.b $EA                   ; $818287
    ror a                       ; $818289
    asl.b $ED                   ; $81828A
    ror a                       ; $81828C
    lsr.b $EE                   ; $81828D
    ror a                       ; $81828F
    sta.b $0E                   ; $818290
    asl.b $F1                   ; $818292
    ror a                       ; $818294
    lsr.b $F2                   ; $818295
    ror a                       ; $818297
    asl.b $F5                   ; $818298
    ror a                       ; $81829A
    lsr.b $F6                   ; $81829B
    ror a                       ; $81829D
    asl.b $F9                   ; $81829E
    ror a                       ; $8182A0
    lsr.b $FA                   ; $8182A1
    ror a                       ; $8182A3
    asl.b $FD                   ; $8182A4
    ror a                       ; $8182A6
    lsr.b $FE                   ; $8182A7
    ror a                       ; $8182A9
    sta.b $0F                   ; $8182AA
    rep.b #$20                  ; $8182AC
    tdc                         ; $8182AE
    clc                         ; $8182AF
    adc.w #$0100                ; $8182B0
    tcd                         ; $8182B3
    sep.b #$20                  ; $8182B4
    asl.b $01                   ; $8182B6
    ror a                       ; $8182B8
    lsr.b $02                   ; $8182B9
    ror a                       ; $8182BB
    asl.b $05                   ; $8182BC
    ror a                       ; $8182BE
    lsr.b $06                   ; $8182BF
    ror a                       ; $8182C1
    asl.b $09                   ; $8182C2
    ror a                       ; $8182C4
    lsr.b $0A                   ; $8182C5
    ror a                       ; $8182C7
    asl.b $0D                   ; $8182C8
    ror a                       ; $8182CA
    lsr.b $0E                   ; $8182CB
    ror a                       ; $8182CD
    sta.w $0210,x               ; $8182CE
    asl.b $11                   ; $8182D1
    ror a                       ; $8182D3
    lsr.b $12                   ; $8182D4
    ror a                       ; $8182D6
    asl.b $15                   ; $8182D7
    ror a                       ; $8182D9
    lsr.b $16                   ; $8182DA
    ror a                       ; $8182DC
    asl.b $19                   ; $8182DD
    ror a                       ; $8182DF
    lsr.b $1A                   ; $8182E0
    ror a                       ; $8182E2
    asl.b $1D                   ; $8182E3
    ror a                       ; $8182E5
    lsr.b $1E                   ; $8182E6
    ror a                       ; $8182E8
    sta.w $0211,x               ; $8182E9
    asl.b $21                   ; $8182EC
    ror a                       ; $8182EE
    lsr.b $22                   ; $8182EF
    ror a                       ; $8182F1
    asl.b $25                   ; $8182F2
    ror a                       ; $8182F4
    lsr.b $26                   ; $8182F5
    ror a                       ; $8182F7
    asl.b $29                   ; $8182F8
    ror a                       ; $8182FA
    lsr.b $2A                   ; $8182FB
    ror a                       ; $8182FD
    asl.b $2D                   ; $8182FE
    ror a                       ; $818300
    lsr.b $2E                   ; $818301
    ror a                       ; $818303
    sta.w $0212,x               ; $818304
    asl.b $31                   ; $818307
    ror a                       ; $818309
    lsr.b $32                   ; $81830A
    ror a                       ; $81830C
    asl.b $35                   ; $81830D
    ror a                       ; $81830F
    lsr.b $36                   ; $818310
    ror a                       ; $818312
    asl.b $39                   ; $818313
    ror a                       ; $818315
    lsr.b $3A                   ; $818316
    ror a                       ; $818318
    asl.b $3D                   ; $818319
    ror a                       ; $81831B
    lsr.b $3E                   ; $81831C
    ror a                       ; $81831E
    sta.w $0213,x               ; $81831F
    asl.b $41                   ; $818322
    ror a                       ; $818324
    lsr.b $42                   ; $818325
    ror a                       ; $818327
    asl.b $45                   ; $818328
    ror a                       ; $81832A
    lsr.b $46                   ; $81832B
    ror a                       ; $81832D
    asl.b $49                   ; $81832E
    ror a                       ; $818330
    lsr.b $4A                   ; $818331
    ror a                       ; $818333
    asl.b $4D                   ; $818334
    ror a                       ; $818336
    lsr.b $4E                   ; $818337
    ror a                       ; $818339
    sta.w $0214,x               ; $81833A
    asl.b $51                   ; $81833D
    ror a                       ; $81833F
    lsr.b $52                   ; $818340
    ror a                       ; $818342
    asl.b $55                   ; $818343
    ror a                       ; $818345
    lsr.b $56                   ; $818346
    ror a                       ; $818348
    asl.b $59                   ; $818349
    ror a                       ; $81834B
    lsr.b $5A                   ; $81834C
    ror a                       ; $81834E
    asl.b $5D                   ; $81834F
    ror a                       ; $818351
    lsr.b $5E                   ; $818352
    ror a                       ; $818354
    sta.w $0215,x               ; $818355
    asl.b $61                   ; $818358
    ror a                       ; $81835A
    lsr.b $62                   ; $81835B
    ror a                       ; $81835D
    asl.b $65                   ; $81835E
    ror a                       ; $818360
    lsr.b $66                   ; $818361
    ror a                       ; $818363
    asl.b $69                   ; $818364
    ror a                       ; $818366
    lsr.b $6A                   ; $818367
    ror a                       ; $818369
    asl.b $6D                   ; $81836A
    ror a                       ; $81836C
    lsr.b $6E                   ; $81836D
    ror a                       ; $81836F
    sta.w $0216,x               ; $818370
    asl.b $71                   ; $818373
    ror a                       ; $818375
    lsr.b $72                   ; $818376
    ror a                       ; $818378
    asl.b $75                   ; $818379
    ror a                       ; $81837B
    lsr.b $76                   ; $81837C
    ror a                       ; $81837E
    asl.b $79                   ; $81837F
    ror a                       ; $818381
    lsr.b $7A                   ; $818382
    ror a                       ; $818384
    asl.b $7D                   ; $818385
    ror a                       ; $818387
    lsr.b $7E                   ; $818388
    ror a                       ; $81838A
    sta.w $0217,x               ; $81838B
    asl.b $81                   ; $81838E
    ror a                       ; $818390
    lsr.b $82                   ; $818391
    ror a                       ; $818393
    asl.b $85                   ; $818394
    ror a                       ; $818396
    lsr.b $86                   ; $818397
    ror a                       ; $818399
    asl.b $89                   ; $81839A
    ror a                       ; $81839C
    lsr.b $8A                   ; $81839D
    ror a                       ; $81839F
    asl.b $8D                   ; $8183A0
    ror a                       ; $8183A2
    lsr.b $8E                   ; $8183A3
    ror a                       ; $8183A5
    sta.w $0218,x               ; $8183A6
    asl.b $91                   ; $8183A9
    ror a                       ; $8183AB
    lsr.b $92                   ; $8183AC
    ror a                       ; $8183AE
    asl.b $95                   ; $8183AF
    ror a                       ; $8183B1
    lsr.b $96                   ; $8183B2
    ror a                       ; $8183B4
    asl.b $99                   ; $8183B5
    ror a                       ; $8183B7
    lsr.b $9A                   ; $8183B8
    ror a                       ; $8183BA
    asl.b $9D                   ; $8183BB
    ror a                       ; $8183BD
    lsr.b $9E                   ; $8183BE
    ror a                       ; $8183C0
    sta.w $0219,x               ; $8183C1
    asl.b $A1                   ; $8183C4
    ror a                       ; $8183C6
    lsr.b $A2                   ; $8183C7
    ror a                       ; $8183C9
    asl.b $A5                   ; $8183CA
    ror a                       ; $8183CC
    lsr.b $A6                   ; $8183CD
    ror a                       ; $8183CF
    asl.b $A9                   ; $8183D0
    ror a                       ; $8183D2
    lsr.b $AA                   ; $8183D3
    ror a                       ; $8183D5
    asl.b $AD                   ; $8183D6
    ror a                       ; $8183D8
    lsr.b $AE                   ; $8183D9
    ror a                       ; $8183DB
    sta.w $021A,x               ; $8183DC
    asl.b $B1                   ; $8183DF
    ror a                       ; $8183E1
    lsr.b $B2                   ; $8183E2
    ror a                       ; $8183E4
    asl.b $B5                   ; $8183E5
    ror a                       ; $8183E7
    lsr.b $B6                   ; $8183E8
    ror a                       ; $8183EA
    asl.b $B9                   ; $8183EB
    ror a                       ; $8183ED
    lsr.b $BA                   ; $8183EE
    ror a                       ; $8183F0
    asl.b $BD                   ; $8183F1
    ror a                       ; $8183F3
    lsr.b $BE                   ; $8183F4
    ror a                       ; $8183F6
    sta.w $021B,x               ; $8183F7
    asl.b $C1                   ; $8183FA
    ror a                       ; $8183FC
    lsr.b $C2                   ; $8183FD
    ror a                       ; $8183FF
    asl.b $C5                   ; $818400
    ror a                       ; $818402
    lsr.b $C6                   ; $818403
    ror a                       ; $818405
    asl.b $C9                   ; $818406
    ror a                       ; $818408
    lsr.b $CA                   ; $818409
    ror a                       ; $81840B
    asl.b $CD                   ; $81840C
    ror a                       ; $81840E
    lsr.b $CE                   ; $81840F
    ror a                       ; $818411
    sta.w $021C,x               ; $818412
    asl.b $D1                   ; $818415
    ror a                       ; $818417
    lsr.b $D2                   ; $818418
    ror a                       ; $81841A
    asl.b $D5                   ; $81841B
    ror a                       ; $81841D
    lsr.b $D6                   ; $81841E
    ror a                       ; $818420
    asl.b $D9                   ; $818421
    ror a                       ; $818423
    lsr.b $DA                   ; $818424
    ror a                       ; $818426
    asl.b $DD                   ; $818427
    ror a                       ; $818429
    lsr.b $DE                   ; $81842A
    ror a                       ; $81842C
    sta.w $021D,x               ; $81842D
    asl.b $E1                   ; $818430
    ror a                       ; $818432
    lsr.b $E2                   ; $818433
    ror a                       ; $818435
    asl.b $E5                   ; $818436
    ror a                       ; $818438
    lsr.b $E6                   ; $818439
    ror a                       ; $81843B
    asl.b $E9                   ; $81843C
    ror a                       ; $81843E
    lsr.b $EA                   ; $81843F
    ror a                       ; $818441
    asl.b $ED                   ; $818442
    ror a                       ; $818444
    lsr.b $EE                   ; $818445
    ror a                       ; $818447
    sta.w $021E,x               ; $818448
    asl.b $F1                   ; $81844B
    ror a                       ; $81844D
    lsr.b $F2                   ; $81844E
    ror a                       ; $818450
    asl.b $F5                   ; $818451
    ror a                       ; $818453
    lsr.b $F6                   ; $818454
    ror a                       ; $818456
    asl.b $F9                   ; $818457
    ror a                       ; $818459
    lsr.b $FA                   ; $81845A
    ror a                       ; $81845C
    asl.b $FD                   ; $81845D
    ror a                       ; $81845F
    lsr.b $FE                   ; $818460
    ror a                       ; $818462
    sta.w $021F,x               ; $818463
    rep.b #$20                  ; $818466
    pld                         ; $818468
    lda.b $74                   ; $818469
    sec                         ; $81846B
    sbc.b $72                   ; $81846C
    bcs CODE_818486             ; $81846E
    eor.w #$FFFF                ; $818470
    inc a                       ; $818473
    lsr a                       ; $818474
    lsr a                       ; $818475
    tay                         ; $818476
    ldx.b $74                   ; $818477
    lda.w #$E0E0                ; $818479
CODE_81847C:
    sta.w $0900,x               ; $81847C
    inx                         ; $81847F
    inx                         ; $818480
    inx                         ; $818481
    inx                         ; $818482
    dey                         ; $818483
    bne CODE_81847C             ; $818484
CODE_818486:
    rts                         ; $818486
    and.w #$FFFF                ; $818487
    bne CODE_818497             ; $81848A
    incbin !baserom:$848C..$8497
CODE_818497:
    sty.b $32                   ; $818497
    ldy.b $14                   ; $818499
    phy                         ; $81849B
    ldy.b $16                   ; $81849C
    phy                         ; $81849E
    ora.w #$0001                ; $81849F
    sta.b $14                   ; $8184A2
    lsr a                       ; $8184A4
    eor.w #$FFFF                ; $8184A5
    inc a                       ; $8184A8
    sta.b $16                   ; $8184A9
    txa                         ; $8184AB
    clc                         ; $8184AC
    adc.b $16                   ; $8184AD
    sta.b $16                   ; $8184AF
    ldy.b $84                   ; $8184B1
    stz.w $8000                 ; $8184B3
    lda.b $14                   ; $8184B6
    sec                         ; $8184B8
    sbc.w #$0008                ; $8184B9
    sta.b $14                   ; $8184BC
    bmi CODE_8184ED             ; $8184BE
    ldx.w #$20F6                ; $8184C0
    jsr.w $8527                 ; $8184C3
    db $B0,$40 ; bcs            ; $8184C6
    lda.b $16                   ; $8184C8
    clc                         ; $8184CA
    adc.w #$0008                ; $8184CB
    sta.b $16                   ; $8184CE
    lda.b $14                   ; $8184D0
    sec                         ; $8184D2
    sbc.w #$0008                ; $8184D3
    sta.b $14                   ; $8184D6
    bmi CODE_8184ED             ; $8184D8
    ldx.w #$20F7                ; $8184DA
    jsr.w $8527                 ; $8184DD
    db $B0,$26 ; bcs            ; $8184E0
    lda.b $16                   ; $8184E2
    clc                         ; $8184E4
    adc.w #$0008                ; $8184E5
    sta.b $16                   ; $8184E8
    jmp.w $84D0                 ; $8184EA
CODE_8184ED:
    lda.b $14                   ; $8184ED
    clc                         ; $8184EF
    adc.b $16                   ; $8184F0
    sta.b $16                   ; $8184F2
    ldx.w #$60F6                ; $8184F4
    jsr.w $8527                 ; $8184F7
    db $B0,$0C ; bcs            ; $8184FA
    pla                         ; $8184FC
    sta.b $16                   ; $8184FD
    pla                         ; $8184FF
    sta.b $14                   ; $818500
    stz.w $8000                 ; $818502
    sty.b $84                   ; $818505
    rtl                         ; $818507
    incbin !baserom:$8508..$8527
    stx.w $8000                 ; $818527
    lda.w #$FFF8                ; $81852A
    clc                         ; $81852D
    adc.b $32                   ; $81852E
    sta.w $8000                 ; $818530
    lda.b $16                   ; $818533
    sta.w $8000                 ; $818535
CODE_818538:
    bit.w $C000                 ; $818538
    bpl CODE_818538             ; $81853B
    lda.w $8000                 ; $81853D
    beq CODE_818560             ; $818540
CODE_818542:
    iny                         ; $818542
    iny                         ; $818543
    lda.w $8000                 ; $818544
    sta.w $1574,y               ; $818547
    iny                         ; $81854A
    iny                         ; $81854B
    lda.w $8000                 ; $81854C
    sta.w $1574,y               ; $81854F
    cpy.b $1A                   ; $818552
    db $F0,$0C ; beq            ; $818554
CODE_818556:
    bit.w $C000                 ; $818556
    bpl CODE_818556             ; $818559
    lda.w $8000                 ; $81855B
    bne CODE_818542             ; $81855E
CODE_818560:
    clc                         ; $818560
    rts                         ; $818561
    incbin !baserom:$8562..$8564
    pea.w $8330                 ; $818564
    plb                         ; $818567
    ldx.w #$2E2C                ; $818568
    lda.w $00B4                 ; $81856B
    bne CODE_818573             ; $81856E
    ldx.w #$318C                ; $818570
CODE_818573:
    stx.b $16                   ; $818573
    stx.w $1E4E                 ; $818575
    lda.w $0226                 ; $818578
    asl a                       ; $81857B
    and.w #$003E                ; $81857C
    sta.b $1A                   ; $81857F
    pea.w $307E                 ; $818581
    plb                         ; $818584
    ldy.w #$001B                ; $818585
CODE_818588:
    lda.w #$00FF                ; $818588
    sta.w $0000,x               ; $81858B
    sta.w $0004,x               ; $81858E
    sta.w $0008,x               ; $818591
    sta.w $000C,x               ; $818594
    sta.w $0010,x               ; $818597
    sta.w $0014,x               ; $81859A
    sta.w $0018,x               ; $81859D
    sta.w $001C,x               ; $8185A0
    lda.w #$FF00                ; $8185A3
    sta.w $0002,x               ; $8185A6
    sta.w $0006,x               ; $8185A9
    sta.w $000A,x               ; $8185AC
    sta.w $000E,x               ; $8185AF
    sta.w $0012,x               ; $8185B2
    sta.w $0016,x               ; $8185B5
    sta.w $001A,x               ; $8185B8
    sta.w $001E,x               ; $8185BB
    txa                         ; $8185BE
    clc                         ; $8185BF
    adc.w #$0020                ; $8185C0
    tax                         ; $8185C3
    dey                         ; $8185C4
    bne CODE_818588             ; $8185C5
    plb                         ; $8185C7
    ldx.b $1A                   ; $8185C8
    stz.w $8000                 ; $8185CA
    lda.l $7E3C6E,x             ; $8185CD
    sec                         ; $8185D1
    sbc.l $7E3C6C,x             ; $8185D2
    asl a                       ; $8185D6
    sta.w $8000                 ; $8185D7
    lda.w $0225                 ; $8185DA
    lsr a                       ; $8185DD
    and.w #$007F                ; $8185DE
    xba                         ; $8185E1
    sta.w $8000                 ; $8185E2
    sta.b $1E                   ; $8185E5
    lda.w $8000                 ; $8185E7
    lda.w $8000                 ; $8185EA
    clc                         ; $8185ED
    adc.l $7E3C6C,x             ; $8185EE
    sta.b $1C                   ; $8185F2
    stz.w $8000                 ; $8185F4
    lda.l $7E3F6E,x             ; $8185F7
    sec                         ; $8185FB
    sbc.l $7E3F6C,x             ; $8185FC
    asl a                       ; $818600
    sta.w $8000                 ; $818601
    lda.b $1E                   ; $818604
    sta.w $8000                 ; $818606
    nop                         ; $818609
    lda.w $8000                 ; $81860A
    lda.w $8000                 ; $81860D
    clc                         ; $818610
    adc.l $7E3F6C,x             ; $818611
    sta.b $1E                   ; $818615
    lda.w #$0008                ; $818617
    sta.w $8000                 ; $81861A
    lda.w #$00FF                ; $81861D
    sta.w $8000                 ; $818620
    sta.w $8000                 ; $818623
    sta.w $8000                 ; $818626
    sta.w $8000                 ; $818629
    lda.w #$0000                ; $81862C
    sta.w $8000                 ; $81862F
    sta.w $8000                 ; $818632
    sta.w $8000                 ; $818635
    sta.w $8000                 ; $818638
    lda.w #$00A6                ; $81863B
    sta.w $8000                 ; $81863E
    sta.w $8000                 ; $818641
    sta.w $8000                 ; $818644
    sta.w $8000                 ; $818647
    lda.w #$00A5                ; $81864A
    sta.w $8000                 ; $81864D
    sta.w $8000                 ; $818650
    lda.w #$00A7                ; $818653
    sta.w $8000                 ; $818656
    sta.w $8000                 ; $818659
    lda.w #$0081                ; $81865C
    sta.w $8000                 ; $81865F
    lda.w #$007F                ; $818662
    sta.w $8000                 ; $818665
    lda.w #$0081                ; $818668
    sta.w $8000                 ; $81866B
    lda.w #$007F                ; $81866E
    sta.w $8000                 ; $818671
    lda.w $0242                 ; $818674
    bpl CODE_81868A             ; $818677
    incbin !baserom:$8679..$868A
CODE_81868A:
    lda.b $16                   ; $81868A
    clc                         ; $81868C
    adc.w #$035C                ; $81868D
    sta.w $8000                 ; $818690
    sta.w $8000                 ; $818693
    sta.b $18                   ; $818696
    lda.w $1EF2                 ; $818698
    bpl CODE_8186B0             ; $81869B
    incbin !baserom:$869D..$86B0
CODE_8186B0:
    lda.b $16                   ; $8186B0
    clc                         ; $8186B2
    adc.w #$035E                ; $8186B3
    sta.w $8000                 ; $8186B6
    sta.w $8000                 ; $8186B9
    lda.w $0242                 ; $8186BC
    bpl CODE_8186C7             ; $8186BF
    incbin !baserom:$86C1..$86C7
CODE_8186C7:
    lda.w #$0033                ; $8186C7
    sta.w $8000                 ; $8186CA
    sta.w $8000                 ; $8186CD
    lda.w $1EF2                 ; $8186D0
    bpl CODE_8186DB             ; $8186D3
    incbin !baserom:$86D5..$86DB
CODE_8186DB:
    lda.w #$0033                ; $8186DB
    sta.w $8000                 ; $8186DE
    sta.w $8000                 ; $8186E1
    lda.w #$FF5B                ; $8186E4
    sta.w $8000                 ; $8186E7
    sta.w $8000                 ; $8186EA
    sta.w $8000                 ; $8186ED
    sta.w $8000                 ; $8186F0
    lda.w #$2FC8                ; $8186F3
    sta.w $8000                 ; $8186F6
    lda.w #$0034                ; $8186F9
    sta.w $8000                 ; $8186FC
    lda.w #$FF5C                ; $8186FF
    sta.w $8000                 ; $818702
    lda.w #$0035                ; $818705
    sta.w $8000                 ; $818708
    lda.w #$7FFF                ; $81870B
    sta.w $8000                 ; $81870E
    lda.w $0202                 ; $818711
    sta.w $8000                 ; $818714
    lda.w $0242                 ; $818717
    clc                         ; $81871A
    adc.w #$0033                ; $81871B
    sta.w $8000                 ; $81871E
    lda.w $1EF6                 ; $818721
    sta.w $8000                 ; $818724
    lda.w $1EF2                 ; $818727
    clc                         ; $81872A
    adc.w #$0033                ; $81872B
    sta.w $8000                 ; $81872E
    ldx.b $1A                   ; $818731
    lda.l $7E3D6E,x             ; $818733
    lsr a                       ; $818737
    bcc CODE_818746             ; $818738
    lda.l $7E3C6C,x             ; $81873A
    sta.b $1C                   ; $81873E
    lda.l $7E3F6C,x             ; $818740
    sta.b $1E                   ; $818744
CODE_818746:
    lda.b $1C                   ; $818746
    sta.w $8000                 ; $818748
    nop                         ; $81874B
    lda.b $1E                   ; $81874C
    sta.w $8000                 ; $81874E
    nop                         ; $818751
    lda.b $1C                   ; $818752
    sta.w $8000                 ; $818754
    nop                         ; $818757
    lda.b $1E                   ; $818758
    sta.w $8000                 ; $81875A
    ldx.b $18                   ; $81875D
    lda.w #$0000                ; $81875F
    sec                         ; $818762
    sbc.w $0242                 ; $818763
    bmi CODE_818788             ; $818766
    beq CODE_818788             ; $818768
    incbin !baserom:$876A..$8788
CODE_818788:
    bit.w $C000                 ; $818788
    bpl CODE_818788             ; $81878B
    lda.w $8000                 ; $81878D
CODE_818790:
    bit.w $C000                 ; $818790
    bpl CODE_818790             ; $818793
    lda.w #$0002                ; $818795
    sta.b $14                   ; $818798
    ldx.b $14                   ; $81879A
    lda.l $7E38EC,x             ; $81879C
    sta.w $8000                 ; $8187A0
    lda.l $7E35EC,x             ; $8187A3
    sta.w $8000                 ; $8187A7
    lda.l $7E37EC,x             ; $8187AA
    sta.w $8000                 ; $8187AE
    lda.l $7E366C,x             ; $8187B1
    sta.w $8000                 ; $8187B5
    lda.l $7E386C,x             ; $8187B8
    sta.w $8000                 ; $8187BC
    ldx.b $1A                   ; $8187BF
    lda.l $7E3D6E,x             ; $8187C1
    lsr a                       ; $8187C5
    bcc CODE_8187E6             ; $8187C6
    lda.l $7E3C6C,x             ; $8187C8
    sta.w $8000                 ; $8187CC
    lda.l $7E3F6C,x             ; $8187CF
    sta.w $8000                 ; $8187D3
    lda.l $7E3C6C,x             ; $8187D6
    sta.w $8000                 ; $8187DA
    lda.l $7E3F6C,x             ; $8187DD
    sta.w $8000                 ; $8187E1
    bra CODE_818802             ; $8187E4
CODE_8187E6:
    lda.l $7E3C6E,x             ; $8187E6
    sta.w $8000                 ; $8187EA
    lda.l $7E3F6E,x             ; $8187ED
    sta.w $8000                 ; $8187F1
    lda.l $7E3C6E,x             ; $8187F4
    sta.w $8000                 ; $8187F8
    lda.l $7E3F6E,x             ; $8187FB
    sta.w $8000                 ; $8187FF
CODE_818802:
    jsr.w $89F5                 ; $818802
    ldx.b $1A                   ; $818805
    lda.l $7E3D6E,x             ; $818807
    lsr a                       ; $81880B
    bcc CODE_818854             ; $81880C
    ldx.b $14                   ; $81880E
    lda.l $7E38EC,x             ; $818810
    sta.w $8000                 ; $818814
    lda.l $7E35EC,x             ; $818817
    sta.w $8000                 ; $81881B
    lda.l $7E37EC,x             ; $81881E
    sta.w $8000                 ; $818822
    lda.l $7E366C,x             ; $818825
    sta.w $8000                 ; $818829
    lda.l $7E386C,x             ; $81882C
    sta.w $8000                 ; $818830
    ldx.b $1A                   ; $818833
    lda.l $7E3C6E,x             ; $818835
    sta.w $8000                 ; $818839
    lda.l $7E3F6E,x             ; $81883C
    sta.w $8000                 ; $818840
    lda.l $7E3C6E,x             ; $818843
    sta.w $8000                 ; $818847
    lda.l $7E3F6E,x             ; $81884A
    sta.w $8000                 ; $81884E
    jsr.w $89F5                 ; $818851
CODE_818854:
    ldx.b $14                   ; $818854
    inc.b $1A                   ; $818856
    inc.b $1A                   ; $818858
    inx                         ; $81885A
    inx                         ; $81885B
    cpx.w $0190                 ; $81885C
    stx.b $14                   ; $81885F
    beq CODE_818866             ; $818861
    jmp.w $879C                 ; $818863
CODE_818866:
    lda.w #$8000                ; $818866
    sta.w $8000                 ; $818869
    lda.w $8000                 ; $81886C
    db $D0,$5A ; bne            ; $81886F
    stz.b $C8                   ; $818871
    sep.b #$20                  ; $818873
    lda.b #$C2                  ; $818875
    sta.b $B6                   ; $818877
    lda.b #$B0                  ; $818879
    sta.b $B7                   ; $81887B
    lda.b #$A1                  ; $81887D
    sta.b $B8                   ; $81887F
    lda.l $7E4E55               ; $818881
    sta.b $B9                   ; $818885
    lda.l $7E4E57               ; $818887
    sta.b $BA                   ; $81888B
    lda.l $7E4E59               ; $81888D
    sta.b $BB                   ; $818891
    rep.b #$20                  ; $818893
    lda.w #$3FFF                ; $818895
    sta.b $14                   ; $818898
    lda.w #$3FFF                ; $81889A
    sta.b $16                   ; $81889D
    ldx.w #$0002                ; $81889F
CODE_8188A2:
    lda.l $7E386C,x             ; $8188A2
    cmp.b $14                   ; $8188A6
    beq CODE_8188AC             ; $8188A8
    bpl CODE_8188B0             ; $8188AA
CODE_8188AC:
    sta.b $14                   ; $8188AC
    stx.b $18                   ; $8188AE
CODE_8188B0:
    lda.l $7E37EC,x             ; $8188B0
    cmp.b $16                   ; $8188B4
    beq CODE_8188BA             ; $8188B6
    bpl CODE_8188BE             ; $8188B8
CODE_8188BA:
    sta.b $16                   ; $8188BA
    stx.b $1A                   ; $8188BC
CODE_8188BE:
    inx                         ; $8188BE
    inx                         ; $8188BF
    cpx.w $0194                 ; $8188C0
    bne CODE_8188A2             ; $8188C3
    jmp.w $891A                 ; $8188C5
CODE_8188C8:
    jmp.w $89E1                 ; $8188C8
    incbin !baserom:$88CB..$891A
    lda.b $16                   ; $81891A
    cmp.b $14                   ; $81891C
    bpl CODE_8188C8             ; $81891E
    incbin !baserom:$8920..$89E1
    lda.w $00EA                 ; $8189E1
    clc                         ; $8189E4
    adc.w #$0011                ; $8189E5
    cmp.w #$A422                ; $8189E8
    bne CODE_8189F0             ; $8189EB
    lda.w #$A411                ; $8189ED
CODE_8189F0:
    sta.w $00EA                 ; $8189F0
    plb                         ; $8189F3
    rtl                         ; $8189F4
CODE_8189F5:
    lda.w $C000                 ; $8189F5
    bpl CODE_8189F5             ; $8189F8
    ldy.w $8000                 ; $8189FA
    beq CODE_818A1F             ; $8189FD
    nop                         ; $8189FF
    nop                         ; $818A00
    dey                         ; $818A01
    beq CODE_818A13             ; $818A02
CODE_818A04:
    ldx.w $8000                 ; $818A04
    nop                         ; $818A07
    nop                         ; $818A08
    lda.w $8000                 ; $818A09
    sta.l $7E0000,x             ; $818A0C
    dey                         ; $818A10
    bne CODE_818A04             ; $818A11
CODE_818A13:
    ldx.w $8000                 ; $818A13
    nop                         ; $818A16
    nop                         ; $818A17
    lda.w $8000                 ; $818A18
    sta.l $7E0000,x             ; $818A1B
CODE_818A1F:
    lda.w $C000                 ; $818A1F
    bpl CODE_818A1F             ; $818A22
    ldy.w $8000                 ; $818A24
    beq CODE_818A49             ; $818A27
    nop                         ; $818A29
    nop                         ; $818A2A
    dey                         ; $818A2B
    beq CODE_818A3D             ; $818A2C
CODE_818A2E:
    ldx.w $8000                 ; $818A2E
    nop                         ; $818A31
    nop                         ; $818A32
    lda.w $8000                 ; $818A33
    sta.l $7E0000,x             ; $818A36
    dey                         ; $818A3A
    bne CODE_818A2E             ; $818A3B
CODE_818A3D:
    ldx.w $8000                 ; $818A3D
    nop                         ; $818A40
    nop                         ; $818A41
    lda.w $8000                 ; $818A42
    sta.l $7E0000,x             ; $818A45
CODE_818A49:
    rts                         ; $818A49
    incbin !baserom:$8A4A..$E631
    lda.w $019A                 ; $81E631
    asl a                       ; $81E634
    sta.b $1E                   ; $81E635
    asl a                       ; $81E637
    sbc.w #$0003                ; $81E638
    sta.b $14                   ; $81E63B
    lda.w #$C001                ; $81E63D
    sta.b $16                   ; $81E640
    sta.b $1A                   ; $81E642
    lda.w #$3FFF                ; $81E644
    sta.b $18                   ; $81E647
    sta.b $1C                   ; $81E649
    lda.w #$0000                ; $81E64B
    sta.l $308000               ; $81E64E
    lda.w $019A                 ; $81E652
    asl a                       ; $81E655
    asl a                       ; $81E656
    xba                         ; $81E657
    and.w #$00FF                ; $81E658
    dec a                       ; $81E65B
    sta.l $308000               ; $81E65C
    lda.w #$000A                ; $81E660
    sta.l $308000               ; $81E663
    nop                         ; $81E667
    lda.l $308000               ; $81E668
    clc                         ; $81E66C
    adc.b $98                   ; $81E66D
    sta.b $2A                   ; $81E66F
    lda.l $308000               ; $81E671
    lda.w #$0040                ; $81E675
    sta.b $30                   ; $81E678
    lda.w #$00DB                ; $81E67A
    sta.b $20                   ; $81E67D
    sta.b $22                   ; $81E67F
    ldy.w #$0000                ; $81E681
    ldx.b $2A                   ; $81E684
    lda.l $840004,x             ; $81E686
    cmp.w #$F8F3                ; $81E68A
    bne CODE_81E692             ; $81E68D
    ldy.w #$0002                ; $81E68F
CODE_81E692:
    lda.w $1F28,y               ; $81E692
    sta.b $20                   ; $81E695
    sta.b $22                   ; $81E697
    dec.b $30                   ; $81E699
    bne CODE_81E6AA             ; $81E69B
    lda.b $2A                   ; $81E69D
    sec                         ; $81E69F
    sbc.w #$000A                ; $81E6A0
    sta.b $2A                   ; $81E6A3
    lda.w #$0040                ; $81E6A5
    sta.b $30                   ; $81E6A8
CODE_81E6AA:
    lda.b $16                   ; $81E6AA
    sec                         ; $81E6AC
    sbc.w #$0020                ; $81E6AD
    eor.w #$FFFF                ; $81E6B0
    inc a                       ; $81E6B3
    cmp.b $20                   ; $81E6B4
    bmi CODE_81E6BA             ; $81E6B6
    lda.b $20                   ; $81E6B8
CODE_81E6BA:
    eor.w #$FFFF                ; $81E6BA
    inc a                       ; $81E6BD
    sta.b $16                   ; $81E6BE
    lda.b $18                   ; $81E6C0
    clc                         ; $81E6C2
    adc.w #$0008                ; $81E6C3
    cmp.b $20                   ; $81E6C6
    bmi CODE_81E6CC             ; $81E6C8
    lda.b $20                   ; $81E6CA
CODE_81E6CC:
    sta.b $18                   ; $81E6CC
    lda.b $1A                   ; $81E6CE
    sec                         ; $81E6D0
    sbc.w #$0008                ; $81E6D1
    eor.w #$FFFF                ; $81E6D4
    inc a                       ; $81E6D7
    cmp.b $22                   ; $81E6D8
    bmi CODE_81E6DE             ; $81E6DA
    lda.b $22                   ; $81E6DC
CODE_81E6DE:
    eor.w #$FFFF                ; $81E6DE
    inc a                       ; $81E6E1
    sta.b $1A                   ; $81E6E2
    lda.b $1C                   ; $81E6E4
    clc                         ; $81E6E6
    adc.w #$0020                ; $81E6E7
    cmp.b $22                   ; $81E6EA
    bmi CODE_81E6F0             ; $81E6EC
    lda.b $22                   ; $81E6EE
CODE_81E6F0:
    sta.b $1C                   ; $81E6F0
    ldx.b $14                   ; $81E6F2
    lda.l $7E8CB4,x             ; $81E6F4
    and.w #$00FF                ; $81E6F8
    beq CODE_81E728             ; $81E6FB
    asl a                       ; $81E6FD
    tax                         ; $81E6FE
    jsr.w ($7FFE,x)             ; $81E6FF
    lda.b $18                   ; $81E702
    sec                         ; $81E704
    sbc.b $26                   ; $81E705
    sta.b $28                   ; $81E707
    lda.b $24                   ; $81E709
    sec                         ; $81E70B
    sbc.b $16                   ; $81E70C
    cmp.b $28                   ; $81E70E
    bmi CODE_81E71E             ; $81E710
    lda.b $24                   ; $81E712
    cmp.b $18                   ; $81E714
    bpl CODE_81E728             ; $81E716
    lda.b $24                   ; $81E718
    sta.b $18                   ; $81E71A
    bra CODE_81E728             ; $81E71C
CODE_81E71E:
    lda.b $16                   ; $81E71E
    cmp.b $26                   ; $81E720
    bpl CODE_81E728             ; $81E722
    lda.b $26                   ; $81E724
    sta.b $16                   ; $81E726
CODE_81E728:
    lda.w #$0011                ; $81E728
    sta.l $308000               ; $81E72B
    nop                         ; $81E72F
    lda.b $16                   ; $81E730
    sta.l $308000               ; $81E732
    nop                         ; $81E736
    lda.b $18                   ; $81E737
    sta.l $308000               ; $81E739
    nop                         ; $81E73D
    lda.b $1A                   ; $81E73E
    sta.l $308000               ; $81E740
    nop                         ; $81E744
    lda.b $1C                   ; $81E745
    sta.l $308000               ; $81E747
    lda.b $14                   ; $81E74B
    sec                         ; $81E74D
    sbc.w #$0004                ; $81E74E
    sta.b $14                   ; $81E751
    ldx.b $1E                   ; $81E753
    sec                         ; $81E755
    lda.l $308000               ; $81E756
    sta.b $28                   ; $81E75A
    bpl CODE_81E764             ; $81E75C
    clc                         ; $81E75E
    adc.w #$1000                ; $81E75F
    sta.b $28                   ; $81E762
CODE_81E764:
    bit.w #$0800                ; $81E764
    beq CODE_81E77B             ; $81E767
    and.w #$F0FF                ; $81E769
    sta.b $24                   ; $81E76C
    lda.b $28                   ; $81E76E
    clc                         ; $81E770
    adc.w #$0100                ; $81E771
    and.w #$0F00                ; $81E774
    ora.b $24                   ; $81E777
    sta.b $28                   ; $81E779
CODE_81E77B:
    bit.w #$0080                ; $81E77B
    beq CODE_81E792             ; $81E77E
    and.w #$FF0F                ; $81E780
    sta.b $24                   ; $81E783
    lda.b $28                   ; $81E785
    clc                         ; $81E787
    adc.w #$0010                ; $81E788
    and.w #$00F0                ; $81E78B
    ora.b $24                   ; $81E78E
    sta.b $28                   ; $81E790
CODE_81E792:
    bit.w #$0008                ; $81E792
    beq CODE_81E7A7             ; $81E795
    incbin !baserom:$E797..$E7A7
CODE_81E7A7:
    sta.l $7ED1B1,x             ; $81E7A7
    dex                         ; $81E7AB
    dex                         ; $81E7AC
    stx.b $1E                   ; $81E7AD
    beq CODE_81E7B4             ; $81E7AF
    jmp.w $E681                 ; $81E7B1
CODE_81E7B4:
    lda.w $019A                 ; $81E7B4
    asl a                       ; $81E7B7
    tax                         ; $81E7B8
    ldx.w #$D1B3                ; $81E7B9
    lda.w $019A                 ; $81E7BC
    asl a                       ; $81E7BF
    clc                         ; $81E7C0
    adc.w #$D1B3                ; $81E7C1
    tay                         ; $81E7C4
    lda.w #$000F                ; $81E7C5
    phb                         ; $81E7C8
    mvn $7E,$7E                 ; $81E7C9
    plb                         ; $81E7CC
    rtl                         ; $81E7CD
    lda.w #$7FFF                ; $81E7CE
    sta.b $24                   ; $81E7D1
    sta.b $26                   ; $81E7D3
    sta.b $2C                   ; $81E7D5
    sta.b $2E                   ; $81E7D7
    rts                         ; $81E7D9
    ldx.b $14                   ; $81E7DA
    lda.l $7E8CB3,x             ; $81E7DC
    and.w #$00FF                ; $81E7E0
    asl a                       ; $81E7E3
    asl a                       ; $81E7E4
    asl a                       ; $81E7E5
    asl a                       ; $81E7E6
    bit.w #$0800                ; $81E7E7
    beq CODE_81E7EF             ; $81E7EA
    ora.w #$F000                ; $81E7EC
CODE_81E7EF:
    sta.b $28                   ; $81E7EF
    lda.l $7E8CB4,x             ; $81E7F1
    and.w #$00FF                ; $81E7F5
    asl a                       ; $81E7F8
    asl a                       ; $81E7F9
    tax                         ; $81E7FA
    lda.l $8380D0,x             ; $81E7FB
    clc                         ; $81E7FF
    adc.b $28                   ; $81E800
    sta.b $24                   ; $81E802
    sta.b $2C                   ; $81E804
    lda.l $8380D2,x             ; $81E806
    clc                         ; $81E80A
    adc.b $28                   ; $81E80B
    sta.b $26                   ; $81E80D
    sta.b $2E                   ; $81E80F
    rts                         ; $81E811
    pea.w $837E                 ; $81E812
    plb                         ; $81E815
    lda.w $1E8E                 ; $81E816
    ldx.w $1E90                 ; $81E819
    sta.w $FACF,x               ; $81E81C
    lda.w $1E6A                 ; $81E81F
    sta.w $FACD,x               ; $81E822
    ldy.w #$0024                ; $81E825
CODE_81E828:
    ldx.w $1E6A,y               ; $81E828
    lda.w $1E68,y               ; $81E82B
    sta.w $FACF,x               ; $81E82E
    lda.w $1E6C,y               ; $81E831
    sta.w $FACD,x               ; $81E834
    dey                         ; $81E837
    dey                         ; $81E838
    bne CODE_81E828             ; $81E839
    lda.w $1E90                 ; $81E83B
    ldx.w $1E6A                 ; $81E83E
    sta.w $FACF,x               ; $81E841
    lda.w $1E6C                 ; $81E844
    sta.w $FACD,x               ; $81E847
    ldy.w #$0000                ; $81E84A
    lda.w $FABB                 ; $81E84D
    sta.w $FAD5                 ; $81E850
    lda.w $FAB5                 ; $81E853
    sta.w $FAC1                 ; $81E856
    lda.w $FAB7                 ; $81E859
    sta.w $FAC3                 ; $81E85C
    stz.w $FAC7                 ; $81E85F
    ldx.w $FAC5                 ; $81E862
    jsr.w ($F876,x)             ; $81E865
    ldy.w #$0035                ; $81E868
    lda.w $FAF0                 ; $81E86B
    sta.w $FB0A                 ; $81E86E
    lda.w $FAEA                 ; $81E871
    sta.w $FAF6                 ; $81E874
    lda.w $FAEC                 ; $81E877
    sta.w $FAF8                 ; $81E87A
    stz.w $FAFC                 ; $81E87D
    ldx.w $FAFA                 ; $81E880
    jsr.w ($F876,x)             ; $81E883
    ldy.w #$006A                ; $81E886
    lda.w $FB25                 ; $81E889
    sta.w $FB3F                 ; $81E88C
    lda.w $FB1F                 ; $81E88F
    sta.w $FB2B                 ; $81E892
    lda.w $FB21                 ; $81E895
    sta.w $FB2D                 ; $81E898
    stz.w $FB31                 ; $81E89B
    ldx.w $FB2F                 ; $81E89E
    jsr.w ($F876,x)             ; $81E8A1
    ldy.w #$009F                ; $81E8A4
    lda.w $FB5A                 ; $81E8A7
    sta.w $FB74                 ; $81E8AA
    lda.w $FB54                 ; $81E8AD
    sta.w $FB60                 ; $81E8B0
    lda.w $FB56                 ; $81E8B3
    sta.w $FB62                 ; $81E8B6
    stz.w $FB66                 ; $81E8B9
    ldx.w $FB64                 ; $81E8BC
    jsr.w ($F876,x)             ; $81E8BF
    ldy.w #$00D4                ; $81E8C2
    lda.w $FB8F                 ; $81E8C5
    sta.w $FBA9                 ; $81E8C8
    lda.w $FB89                 ; $81E8CB
    sta.w $FB95                 ; $81E8CE
    lda.w $FB8B                 ; $81E8D1
    sta.w $FB97                 ; $81E8D4
    stz.w $FB9B                 ; $81E8D7
    ldx.w $FB99                 ; $81E8DA
    jsr.w ($F876,x)             ; $81E8DD
    ldy.w #$0109                ; $81E8E0
    lda.w $FBC4                 ; $81E8E3
    sta.w $FBDE                 ; $81E8E6
    lda.w $FBBE                 ; $81E8E9
    sta.w $FBCA                 ; $81E8EC
    lda.w $FBC0                 ; $81E8EF
    sta.w $FBCC                 ; $81E8F2
    stz.w $FBD0                 ; $81E8F5
    ldx.w $FBCE                 ; $81E8F8
    jsr.w ($F876,x)             ; $81E8FB
    ldy.w #$013E                ; $81E8FE
    lda.w $FBF9                 ; $81E901
    sta.w $FC13                 ; $81E904
    lda.w $FBF3                 ; $81E907
    sta.w $FBFF                 ; $81E90A
    lda.w $FBF5                 ; $81E90D
    sta.w $FC01                 ; $81E910
    stz.w $FC05                 ; $81E913
    ldx.w $FC03                 ; $81E916
    jsr.w ($F876,x)             ; $81E919
    ldy.w #$0173                ; $81E91C
    lda.w $FC2E                 ; $81E91F
    sta.w $FC48                 ; $81E922
    lda.w $FC28                 ; $81E925
    sta.w $FC34                 ; $81E928
    lda.w $FC2A                 ; $81E92B
    sta.w $FC36                 ; $81E92E
    stz.w $FC3A                 ; $81E931
    ldx.w $FC38                 ; $81E934
    jsr.w ($F876,x)             ; $81E937
    ldy.w #$01A8                ; $81E93A
    lda.w $FC63                 ; $81E93D
    sta.w $FC7D                 ; $81E940
    lda.w $FC5D                 ; $81E943
    sta.w $FC69                 ; $81E946
    lda.w $FC5F                 ; $81E949
    sta.w $FC6B                 ; $81E94C
    stz.w $FC6F                 ; $81E94F
    ldx.w $FC6D                 ; $81E952
    jsr.w ($F876,x)             ; $81E955
    ldy.w #$01DD                ; $81E958
    lda.w $FC98                 ; $81E95B
    sta.w $FCB2                 ; $81E95E
    lda.w $FC92                 ; $81E961
    sta.w $FC9E                 ; $81E964
    lda.w $FC94                 ; $81E967
    sta.w $FCA0                 ; $81E96A
    stz.w $FCA4                 ; $81E96D
    ldx.w $FCA2                 ; $81E970
    jsr.w ($F876,x)             ; $81E973
    ldy.w #$0212                ; $81E976
    lda.w $FCCD                 ; $81E979
    sta.w $FCE7                 ; $81E97C
    lda.w $FCC7                 ; $81E97F
    sta.w $FCD3                 ; $81E982
    lda.w $FCC9                 ; $81E985
    sta.w $FCD5                 ; $81E988
    stz.w $FCD9                 ; $81E98B
    ldx.w $FCD7                 ; $81E98E
    jsr.w ($F876,x)             ; $81E991
    ldy.w #$0247                ; $81E994
    lda.w $FD02                 ; $81E997
    sta.w $FD1C                 ; $81E99A
    lda.w $FCFC                 ; $81E99D
    sta.w $FD08                 ; $81E9A0
    lda.w $FCFE                 ; $81E9A3
    sta.w $FD0A                 ; $81E9A6
    stz.w $FD0E                 ; $81E9A9
    ldx.w $FD0C                 ; $81E9AC
    jsr.w ($F876,x)             ; $81E9AF
    ldy.w #$027C                ; $81E9B2
    lda.w $FD37                 ; $81E9B5
    sta.w $FD51                 ; $81E9B8
    lda.w $FD31                 ; $81E9BB
    sta.w $FD3D                 ; $81E9BE
    lda.w $FD33                 ; $81E9C1
    sta.w $FD3F                 ; $81E9C4
    stz.w $FD43                 ; $81E9C7
    ldx.w $FD41                 ; $81E9CA
    jsr.w ($F876,x)             ; $81E9CD
    ldy.w #$02B1                ; $81E9D0
    lda.w $FD6C                 ; $81E9D3
    sta.w $FD86                 ; $81E9D6
    lda.w $FD66                 ; $81E9D9
    sta.w $FD72                 ; $81E9DC
    lda.w $FD68                 ; $81E9DF
    sta.w $FD74                 ; $81E9E2
    stz.w $FD78                 ; $81E9E5
    ldx.w $FD76                 ; $81E9E8
    jsr.w ($F876,x)             ; $81E9EB
    ldy.w #$02E6                ; $81E9EE
    lda.w $FDA1                 ; $81E9F1
    sta.w $FDBB                 ; $81E9F4
    lda.w $FD9B                 ; $81E9F7
    sta.w $FDA7                 ; $81E9FA
    lda.w $FD9D                 ; $81E9FD
    sta.w $FDA9                 ; $81EA00
    stz.w $FDAD                 ; $81EA03
    ldx.w $FDAB                 ; $81EA06
    jsr.w ($F876,x)             ; $81EA09
    ldy.w #$031B                ; $81EA0C
    lda.w $FDD6                 ; $81EA0F
    sta.w $FDF0                 ; $81EA12
    lda.w $FDD0                 ; $81EA15
    sta.w $FDDC                 ; $81EA18
    lda.w $FDD2                 ; $81EA1B
    sta.w $FDDE                 ; $81EA1E
    stz.w $FDE2                 ; $81EA21
    ldx.w $FDE0                 ; $81EA24
    jsr.w ($F876,x)             ; $81EA27
    ldy.w #$0350                ; $81EA2A
    lda.w $FE0B                 ; $81EA2D
    sta.w $FE25                 ; $81EA30
    lda.w $FE05                 ; $81EA33
    sta.w $FE11                 ; $81EA36
    lda.w $FE07                 ; $81EA39
    sta.w $FE13                 ; $81EA3C
    stz.w $FE17                 ; $81EA3F
    ldx.w $FE15                 ; $81EA42
    jsr.w ($F876,x)             ; $81EA45
    ldy.w #$0385                ; $81EA48
    lda.w $FE40                 ; $81EA4B
    sta.w $FE5A                 ; $81EA4E
    lda.w $FE3A                 ; $81EA51
    sta.w $FE46                 ; $81EA54
    lda.w $FE3C                 ; $81EA57
    sta.w $FE48                 ; $81EA5A
    stz.w $FE4C                 ; $81EA5D
    ldx.w $FE4A                 ; $81EA60
    jsr.w ($F876,x)             ; $81EA63
    ldy.w #$03BA                ; $81EA66
    lda.w $FE75                 ; $81EA69
    sta.w $FE8F                 ; $81EA6C
    lda.w $FE6F                 ; $81EA6F
    sta.w $FE7B                 ; $81EA72
    lda.w $FE71                 ; $81EA75
    sta.w $FE7D                 ; $81EA78
    stz.w $FE81                 ; $81EA7B
    ldx.w $FE7F                 ; $81EA7E
    jsr.w ($F876,x)             ; $81EA81
    ldy.w #$03EF                ; $81EA84
    lda.w $FEAA                 ; $81EA87
    sta.w $FEC4                 ; $81EA8A
    lda.w $FEA4                 ; $81EA8D
    sta.w $FEB0                 ; $81EA90
    lda.w $FEA6                 ; $81EA93
    sta.w $FEB2                 ; $81EA96
    stz.w $FEB6                 ; $81EA99
    ldx.w $FEB4                 ; $81EA9C
    jsr.w ($F876,x)             ; $81EA9F
    stz.b $3C                   ; $81EAA2
    ldx.w $1E8E                 ; $81EAA4
CODE_81EAA7:
    ldy.w $1E90                 ; $81EAA7
    lda.w $FAB5,y               ; $81EAAA
    cmp.w $FAB5,x               ; $81EAAD
    lda.w $FAB7,y               ; $81EAB0
    sbc.w $FAB7,x               ; $81EAB3
    beq CODE_81EACC             ; $81EAB6
    stz.b $3C                   ; $81EAB8
    bpl CODE_81EACF             ; $81EABA
    jsr.w func_81F251           ; $81EABC
    bcs CODE_81EACF             ; $81EABF
    sty.w $1E8E                 ; $81EAC1
    stx.w $1E90                 ; $81EAC4
    jsr.w func_81F1FC           ; $81EAC7
    bra CODE_81EACF             ; $81EACA
CODE_81EACC:
    jsr.w $F21E                 ; $81EACC
CODE_81EACF:
    ldx.w $1E8C                 ; $81EACF
CODE_81EAD2:
    ldy.w $1E8E                 ; $81EAD2
    lda.w $FAB5,y               ; $81EAD5
    cmp.w $FAB5,x               ; $81EAD8
    lda.w $FAB7,y               ; $81EADB
    sbc.w $FAB7,x               ; $81EADE
    beq CODE_81EAF7             ; $81EAE1
    stz.b $3C                   ; $81EAE3
    bpl CODE_81EAFA             ; $81EAE5
    jsr.w func_81F251           ; $81EAE7
    bcs CODE_81EAFA             ; $81EAEA
    sty.w $1E8C                 ; $81EAEC
    stx.w $1E8E                 ; $81EAEF
    jsr.w func_81F1FC           ; $81EAF2
    bra CODE_81EAA7             ; $81EAF5
CODE_81EAF7:
    jsr.w $F21E                 ; $81EAF7
CODE_81EAFA:
    ldx.w $1E8A                 ; $81EAFA
CODE_81EAFD:
    ldy.w $1E8C                 ; $81EAFD
    lda.w $FAB5,y               ; $81EB00
    cmp.w $FAB5,x               ; $81EB03
    lda.w $FAB7,y               ; $81EB06
    sbc.w $FAB7,x               ; $81EB09
    beq CODE_81EB22             ; $81EB0C
    stz.b $3C                   ; $81EB0E
    bpl CODE_81EB25             ; $81EB10
    jsr.w func_81F251           ; $81EB12
    bcs CODE_81EB25             ; $81EB15
    sty.w $1E8A                 ; $81EB17
    stx.w $1E8C                 ; $81EB1A
    jsr.w func_81F1FC           ; $81EB1D
    bra CODE_81EAD2             ; $81EB20
CODE_81EB22:
    jsr.w $F21E                 ; $81EB22
CODE_81EB25:
    ldx.w $1E88                 ; $81EB25
CODE_81EB28:
    ldy.w $1E8A                 ; $81EB28
    lda.w $FAB5,y               ; $81EB2B
    cmp.w $FAB5,x               ; $81EB2E
    lda.w $FAB7,y               ; $81EB31
    sbc.w $FAB7,x               ; $81EB34
    beq CODE_81EB4D             ; $81EB37
    stz.b $3C                   ; $81EB39
    bpl CODE_81EB50             ; $81EB3B
    jsr.w func_81F251           ; $81EB3D
    bcs CODE_81EB50             ; $81EB40
    sty.w $1E88                 ; $81EB42
    stx.w $1E8A                 ; $81EB45
    jsr.w func_81F1FC           ; $81EB48
    bra CODE_81EAFD             ; $81EB4B
CODE_81EB4D:
    jsr.w $F21E                 ; $81EB4D
CODE_81EB50:
    ldx.w $1E86                 ; $81EB50
CODE_81EB53:
    ldy.w $1E88                 ; $81EB53
    lda.w $FAB5,y               ; $81EB56
    cmp.w $FAB5,x               ; $81EB59
    lda.w $FAB7,y               ; $81EB5C
    sbc.w $FAB7,x               ; $81EB5F
    beq CODE_81EB78             ; $81EB62
    stz.b $3C                   ; $81EB64
    bpl CODE_81EB7B             ; $81EB66
    jsr.w func_81F251           ; $81EB68
    bcs CODE_81EB7B             ; $81EB6B
    sty.w $1E86                 ; $81EB6D
    stx.w $1E88                 ; $81EB70
    jsr.w func_81F1FC           ; $81EB73
    bra CODE_81EB28             ; $81EB76
CODE_81EB78:
    jsr.w $F21E                 ; $81EB78
CODE_81EB7B:
    ldx.w $1E84                 ; $81EB7B
CODE_81EB7E:
    ldy.w $1E86                 ; $81EB7E
    lda.w $FAB5,y               ; $81EB81
    cmp.w $FAB5,x               ; $81EB84
    lda.w $FAB7,y               ; $81EB87
    sbc.w $FAB7,x               ; $81EB8A
    beq CODE_81EBA3             ; $81EB8D
    stz.b $3C                   ; $81EB8F
    bpl CODE_81EBA6             ; $81EB91
    jsr.w func_81F251           ; $81EB93
    bcs CODE_81EBA6             ; $81EB96
    sty.w $1E84                 ; $81EB98
    stx.w $1E86                 ; $81EB9B
    jsr.w func_81F1FC           ; $81EB9E
    bra CODE_81EB53             ; $81EBA1
CODE_81EBA3:
    jsr.w $F21E                 ; $81EBA3
CODE_81EBA6:
    ldx.w $1E82                 ; $81EBA6
CODE_81EBA9:
    ldy.w $1E84                 ; $81EBA9
    lda.w $FAB5,y               ; $81EBAC
    cmp.w $FAB5,x               ; $81EBAF
    lda.w $FAB7,y               ; $81EBB2
    sbc.w $FAB7,x               ; $81EBB5
    beq CODE_81EBCE             ; $81EBB8
    stz.b $3C                   ; $81EBBA
    bpl CODE_81EBD1             ; $81EBBC
    jsr.w func_81F251           ; $81EBBE
    bcs CODE_81EBD1             ; $81EBC1
    sty.w $1E82                 ; $81EBC3
    stx.w $1E84                 ; $81EBC6
    jsr.w func_81F1FC           ; $81EBC9
    bra CODE_81EB7E             ; $81EBCC
CODE_81EBCE:
    jsr.w $F21E                 ; $81EBCE
CODE_81EBD1:
    ldx.w $1E80                 ; $81EBD1
CODE_81EBD4:
    ldy.w $1E82                 ; $81EBD4
    lda.w $FAB5,y               ; $81EBD7
    cmp.w $FAB5,x               ; $81EBDA
    lda.w $FAB7,y               ; $81EBDD
    sbc.w $FAB7,x               ; $81EBE0
    beq CODE_81EBF9             ; $81EBE3
    stz.b $3C                   ; $81EBE5
    bpl CODE_81EBFC             ; $81EBE7
    jsr.w func_81F251           ; $81EBE9
    bcs CODE_81EBFC             ; $81EBEC
    sty.w $1E80                 ; $81EBEE
    stx.w $1E82                 ; $81EBF1
    jsr.w func_81F1FC           ; $81EBF4
    bra CODE_81EBA9             ; $81EBF7
CODE_81EBF9:
    jsr.w $F21E                 ; $81EBF9
CODE_81EBFC:
    ldx.w $1E7E                 ; $81EBFC
CODE_81EBFF:
    ldy.w $1E80                 ; $81EBFF
    lda.w $FAB5,y               ; $81EC02
    cmp.w $FAB5,x               ; $81EC05
    lda.w $FAB7,y               ; $81EC08
    sbc.w $FAB7,x               ; $81EC0B
    beq CODE_81EC24             ; $81EC0E
    stz.b $3C                   ; $81EC10
    bpl CODE_81EC27             ; $81EC12
    jsr.w func_81F251           ; $81EC14
    bcs CODE_81EC27             ; $81EC17
    sty.w $1E7E                 ; $81EC19
    stx.w $1E80                 ; $81EC1C
    jsr.w func_81F1FC           ; $81EC1F
    bra CODE_81EBD4             ; $81EC22
CODE_81EC24:
    jsr.w $F21E                 ; $81EC24
CODE_81EC27:
    ldx.w $1E7C                 ; $81EC27
CODE_81EC2A:
    ldy.w $1E7E                 ; $81EC2A
    lda.w $FAB5,y               ; $81EC2D
    cmp.w $FAB5,x               ; $81EC30
    lda.w $FAB7,y               ; $81EC33
    sbc.w $FAB7,x               ; $81EC36
    beq CODE_81EC4F             ; $81EC39
    stz.b $3C                   ; $81EC3B
    bpl CODE_81EC52             ; $81EC3D
    jsr.w func_81F251           ; $81EC3F
    bcs CODE_81EC52             ; $81EC42
    sty.w $1E7C                 ; $81EC44
    stx.w $1E7E                 ; $81EC47
    jsr.w func_81F1FC           ; $81EC4A
    bra CODE_81EBFF             ; $81EC4D
CODE_81EC4F:
    jsr.w $F21E                 ; $81EC4F
CODE_81EC52:
    ldx.w $1E7A                 ; $81EC52
CODE_81EC55:
    ldy.w $1E7C                 ; $81EC55
    lda.w $FAB5,y               ; $81EC58
    cmp.w $FAB5,x               ; $81EC5B
    lda.w $FAB7,y               ; $81EC5E
    sbc.w $FAB7,x               ; $81EC61
    beq CODE_81EC7A             ; $81EC64
    stz.b $3C                   ; $81EC66
    bpl CODE_81EC7D             ; $81EC68
    jsr.w func_81F251           ; $81EC6A
    bcs CODE_81EC7D             ; $81EC6D
    sty.w $1E7A                 ; $81EC6F
    stx.w $1E7C                 ; $81EC72
    jsr.w func_81F1FC           ; $81EC75
    bra CODE_81EC2A             ; $81EC78
CODE_81EC7A:
    jsr.w $F21E                 ; $81EC7A
CODE_81EC7D:
    ldx.w $1E78                 ; $81EC7D
CODE_81EC80:
    ldy.w $1E7A                 ; $81EC80
    lda.w $FAB5,y               ; $81EC83
    cmp.w $FAB5,x               ; $81EC86
    lda.w $FAB7,y               ; $81EC89
    sbc.w $FAB7,x               ; $81EC8C
    beq CODE_81ECA5             ; $81EC8F
    stz.b $3C                   ; $81EC91
    bpl CODE_81ECA8             ; $81EC93
    jsr.w func_81F251           ; $81EC95
    bcs CODE_81ECA8             ; $81EC98
    sty.w $1E78                 ; $81EC9A
    stx.w $1E7A                 ; $81EC9D
    jsr.w func_81F1FC           ; $81ECA0
    bra CODE_81EC55             ; $81ECA3
CODE_81ECA5:
    jsr.w $F21E                 ; $81ECA5
CODE_81ECA8:
    ldx.w $1E76                 ; $81ECA8
CODE_81ECAB:
    ldy.w $1E78                 ; $81ECAB
    lda.w $FAB5,y               ; $81ECAE
    cmp.w $FAB5,x               ; $81ECB1
    lda.w $FAB7,y               ; $81ECB4
    sbc.w $FAB7,x               ; $81ECB7
    beq CODE_81ECD0             ; $81ECBA
    stz.b $3C                   ; $81ECBC
    bpl CODE_81ECD3             ; $81ECBE
    jsr.w func_81F251           ; $81ECC0
    bcs CODE_81ECD3             ; $81ECC3
    sty.w $1E76                 ; $81ECC5
    stx.w $1E78                 ; $81ECC8
    jsr.w func_81F1FC           ; $81ECCB
    bra CODE_81EC80             ; $81ECCE
CODE_81ECD0:
    jsr.w $F21E                 ; $81ECD0
CODE_81ECD3:
    ldx.w $1E74                 ; $81ECD3
CODE_81ECD6:
    ldy.w $1E76                 ; $81ECD6
    lda.w $FAB5,y               ; $81ECD9
    cmp.w $FAB5,x               ; $81ECDC
    lda.w $FAB7,y               ; $81ECDF
    sbc.w $FAB7,x               ; $81ECE2
    beq CODE_81ECFB             ; $81ECE5
    stz.b $3C                   ; $81ECE7
    bpl CODE_81ECFE             ; $81ECE9
    jsr.w func_81F251           ; $81ECEB
    bcs CODE_81ECFE             ; $81ECEE
    sty.w $1E74                 ; $81ECF0
    stx.w $1E76                 ; $81ECF3
    jsr.w func_81F1FC           ; $81ECF6
    bra CODE_81ECAB             ; $81ECF9
CODE_81ECFB:
    jsr.w $F21E                 ; $81ECFB
CODE_81ECFE:
    ldx.w $1E72                 ; $81ECFE
CODE_81ED01:
    ldy.w $1E74                 ; $81ED01
    lda.w $FAB5,y               ; $81ED04
    cmp.w $FAB5,x               ; $81ED07
    lda.w $FAB7,y               ; $81ED0A
    sbc.w $FAB7,x               ; $81ED0D
    beq CODE_81ED26             ; $81ED10
    stz.b $3C                   ; $81ED12
    bpl CODE_81ED29             ; $81ED14
    jsr.w func_81F251           ; $81ED16
    bcs CODE_81ED29             ; $81ED19
    sty.w $1E72                 ; $81ED1B
    stx.w $1E74                 ; $81ED1E
    jsr.w func_81F1FC           ; $81ED21
    bra CODE_81ECD6             ; $81ED24
CODE_81ED26:
    jsr.w $F21E                 ; $81ED26
CODE_81ED29:
    ldx.w $1E70                 ; $81ED29
CODE_81ED2C:
    ldy.w $1E72                 ; $81ED2C
    lda.w $FAB5,y               ; $81ED2F
    cmp.w $FAB5,x               ; $81ED32
    lda.w $FAB7,y               ; $81ED35
    sbc.w $FAB7,x               ; $81ED38
    beq CODE_81ED51             ; $81ED3B
    stz.b $3C                   ; $81ED3D
    bpl CODE_81ED54             ; $81ED3F
    jsr.w func_81F251           ; $81ED41
    bcs CODE_81ED54             ; $81ED44
    sty.w $1E70                 ; $81ED46
    stx.w $1E72                 ; $81ED49
    jsr.w func_81F1FC           ; $81ED4C
    bra CODE_81ED01             ; $81ED4F
CODE_81ED51:
    jsr.w $F21E                 ; $81ED51
CODE_81ED54:
    ldx.w $1E6E                 ; $81ED54
CODE_81ED57:
    ldy.w $1E70                 ; $81ED57
    lda.w $FAB5,y               ; $81ED5A
    cmp.w $FAB5,x               ; $81ED5D
    lda.w $FAB7,y               ; $81ED60
    sbc.w $FAB7,x               ; $81ED63
    beq CODE_81ED7C             ; $81ED66
    stz.b $3C                   ; $81ED68
    bpl CODE_81ED7F             ; $81ED6A
    jsr.w func_81F251           ; $81ED6C
    bcs CODE_81ED7F             ; $81ED6F
    sty.w $1E6E                 ; $81ED71
    stx.w $1E70                 ; $81ED74
    jsr.w func_81F1FC           ; $81ED77
    bra CODE_81ED2C             ; $81ED7A
CODE_81ED7C:
    jsr.w $F21E                 ; $81ED7C
CODE_81ED7F:
    ldx.w $1E6C                 ; $81ED7F
CODE_81ED82:
    ldy.w $1E6E                 ; $81ED82
    lda.w $FAB5,y               ; $81ED85
    cmp.w $FAB5,x               ; $81ED88
    lda.w $FAB7,y               ; $81ED8B
    sbc.w $FAB7,x               ; $81ED8E
    beq CODE_81EDA7             ; $81ED91
    stz.b $3C                   ; $81ED93
    bpl CODE_81EDAA             ; $81ED95
    jsr.w func_81F251           ; $81ED97
    bcs CODE_81EDAA             ; $81ED9A
    sty.w $1E6C                 ; $81ED9C
    stx.w $1E6E                 ; $81ED9F
    jsr.w func_81F1FC           ; $81EDA2
    bra CODE_81ED57             ; $81EDA5
CODE_81EDA7:
    jsr.w $F21E                 ; $81EDA7
CODE_81EDAA:
    ldx.w $1E6A                 ; $81EDAA
    ldy.w $1E6C                 ; $81EDAD
    lda.w $FAB5,y               ; $81EDB0
    cmp.w $FAB5,x               ; $81EDB3
    lda.w $FAB7,y               ; $81EDB6
    sbc.w $FAB7,x               ; $81EDB9
    beq CODE_81EDD2             ; $81EDBC
    stz.b $3C                   ; $81EDBE
    bpl CODE_81EDDD             ; $81EDC0
    jsr.w func_81F251           ; $81EDC2
    bcs CODE_81EDDD             ; $81EDC5
    sty.w $1E6A                 ; $81EDC7
    stx.w $1E6C                 ; $81EDCA
    jsr.w func_81F1FC           ; $81EDCD
    bra CODE_81ED82             ; $81EDD0
CODE_81EDD2:
    jsr.w $F21E                 ; $81EDD2
    bra CODE_81EDDD             ; $81EDD5
CODE_81EDD7:
    jmp.w $EFA9                 ; $81EDD7
CODE_81EDDA:
    jmp.w $EF10                 ; $81EDDA
CODE_81EDDD:
    sec                         ; $81EDDD
    ldy.w #$0000                ; $81EDDE
    ldx.w $1E90                 ; $81EDE1
    lda.w $FAB6,x               ; $81EDE4
    sbc.w $019A                 ; $81EDE7
    bcc CODE_81EDD7             ; $81EDEA
    sta.w $FAB6,x               ; $81EDEC
    iny                         ; $81EDEF
    ldx.w $1E8E                 ; $81EDF0
    lda.w $FAB6,x               ; $81EDF3
    sbc.w $019A                 ; $81EDF6
    bcc CODE_81EDDA             ; $81EDF9
    sta.w $FAB6,x               ; $81EDFB
    iny                         ; $81EDFE
    ldx.w $1E8C                 ; $81EDFF
    lda.w $FAB6,x               ; $81EE02
    sbc.w $019A                 ; $81EE05
    bcc CODE_81EDDA             ; $81EE08
    sta.w $FAB6,x               ; $81EE0A
    iny                         ; $81EE0D
    ldx.w $1E8A                 ; $81EE0E
    lda.w $FAB6,x               ; $81EE11
    sbc.w $019A                 ; $81EE14
    bcc CODE_81EDDA             ; $81EE17
    incbin !baserom:$EE19..$EF10
    ldx.w $1E90                 ; $81EF10
    lda.w $1E8E                 ; $81EF13
    sta.w $1E90                 ; $81EF16
    lda.w $1E8C                 ; $81EF19
    sta.w $1E8E                 ; $81EF1C
    lda.w $1E8A                 ; $81EF1F
    sta.w $1E8C                 ; $81EF22
    lda.w $1E88                 ; $81EF25
    sta.w $1E8A                 ; $81EF28
    lda.w $1E86                 ; $81EF2B
    sta.w $1E88                 ; $81EF2E
    lda.w $1E84                 ; $81EF31
    sta.w $1E86                 ; $81EF34
    lda.w $1E82                 ; $81EF37
    sta.w $1E84                 ; $81EF3A
    lda.w $1E80                 ; $81EF3D
    sta.w $1E82                 ; $81EF40
    lda.w $1E7E                 ; $81EF43
    sta.w $1E80                 ; $81EF46
    lda.w $1E7C                 ; $81EF49
    sta.w $1E7E                 ; $81EF4C
    lda.w $1E7A                 ; $81EF4F
    sta.w $1E7C                 ; $81EF52
    lda.w $1E78                 ; $81EF55
    sta.w $1E7A                 ; $81EF58
    lda.w $1E76                 ; $81EF5B
    sta.w $1E78                 ; $81EF5E
    lda.w $1E74                 ; $81EF61
    sta.w $1E76                 ; $81EF64
    lda.w $1E72                 ; $81EF67
    sta.w $1E74                 ; $81EF6A
    lda.w $1E70                 ; $81EF6D
    sta.w $1E72                 ; $81EF70
    lda.w $1E6E                 ; $81EF73
    sta.w $1E70                 ; $81EF76
    lda.w $1E6C                 ; $81EF79
    sta.w $1E6E                 ; $81EF7C
    lda.w $1E6A                 ; $81EF7F
    sta.w $1E6C                 ; $81EF82
    stx.w $1E6A                 ; $81EF85
    inc.w $FAD3,x               ; $81EF88
    lda.w $FADE,x               ; $81EF8B
    ora.w #$0001                ; $81EF8E
    sta.w $FADE,x               ; $81EF91
    lda.w $019C                 ; $81EF94
    sta.w $FAE0,x               ; $81EF97
    dey                         ; $81EF9A
    beq CODE_81EFA0             ; $81EF9B
    jmp.w $EF10                 ; $81EF9D
CODE_81EFA0:
    jmp.w $EAA2                 ; $81EFA0
CODE_81EFA3:
    jmp.w $F17D                 ; $81EFA3
    incbin !baserom:$EFA6..$EFA9
    ldy.w #$0000                ; $81EFA9
    ldx.w $1E6A                 ; $81EFAC
    lda.w $FAB6,x               ; $81EFAF
    bpl CODE_81EFA3             ; $81EFB2
    incbin !baserom:$EFB4..$F17D
    ldy.w #$0000                ; $81F17D
CODE_81F180:
    lda.w $1E6A,y               ; $81F180
    tax                         ; $81F183
    lda.w $FADE,x               ; $81F184
    bit.w #$0001                ; $81F187
    beq CODE_81F1B6             ; $81F18A
    bit.w #$0002                ; $81F18C
    bne CODE_81F1B6             ; $81F18F
    lda.w $FAD3,x               ; $81F191
    cmp.w $1F06                 ; $81F194
    bne CODE_81F1B6             ; $81F197
    sep.b #$20                  ; $81F199
    lda.w $FADE,x               ; $81F19B
    ora.b #$04                  ; $81F19E
    sta.w $FADE,x               ; $81F1A0
    lda.w $FADD,x               ; $81F1A3
    ldx.w $1F08                 ; $81F1A6
    sta.w $0446,x               ; $81F1A9
    inx                         ; $81F1AC
    stx.w $1F08                 ; $81F1AD
    rep.b #$20                  ; $81F1B0
    lda.w $1E6A,y               ; $81F1B2
    tax                         ; $81F1B5
CODE_81F1B6:
    lda.w $FADE,x               ; $81F1B6
    and.w #$FF7E                ; $81F1B9
    sta.w $FADE,x               ; $81F1BC
    iny                         ; $81F1BF
    iny                         ; $81F1C0
    cpy.w #$0028                ; $81F1C1
    bne CODE_81F180             ; $81F1C4
    ldx.w $1E90                 ; $81F1C6
    lda.w $1E8E                 ; $81F1C9
    sta.w $FACF,x               ; $81F1CC
    lda.w $1E6A                 ; $81F1CF
    sta.w $FACD,x               ; $81F1D2
    ldy.w #$0024                ; $81F1D5
CODE_81F1D8:
    ldx.w $1E6A,y               ; $81F1D8
    lda.w $1E68,y               ; $81F1DB
    sta.w $FACF,x               ; $81F1DE
    lda.w $1E6C,y               ; $81F1E1
    sta.w $FACD,x               ; $81F1E4
    dey                         ; $81F1E7
    dey                         ; $81F1E8
    bne CODE_81F1D8             ; $81F1E9
    ldx.w $1E6A                 ; $81F1EB
    lda.w $1E90                 ; $81F1EE
    sta.w $FACF,x               ; $81F1F1
    lda.w $1E6C                 ; $81F1F4
    sta.w $FACD,x               ; $81F1F7
    plb                         ; $81F1FA
    rtl                         ; $81F1FB
func_81F1FC:
    phx                         ; $81F1FC
    phy                         ; $81F1FD
    ldx.w #$0000                ; $81F1FE
    lda.w $1E6A,x               ; $81F201
CODE_81F204:
    tay                         ; $81F204
    lda.w $1E6C,x               ; $81F205
    sta.w $FACD,y               ; $81F208
    inx                         ; $81F20B
    inx                         ; $81F20C
    cpx.w #$0026                ; $81F20D
    bne CODE_81F204             ; $81F210
    ldy.w $1E90                 ; $81F212
    lda.w $1E6A                 ; $81F215
    sta.w $FACD,y               ; $81F218
    ply                         ; $81F21B
    plx                         ; $81F21C
    rts                         ; $81F21D
    lda.w $FAB5,y               ; $81F21E
    sec                         ; $81F221
    sbc.w $FAB5,x               ; $81F222
    cmp.w #$0060                ; $81F225
    bcs CODE_81F250             ; $81F228
    inc.b $3C                   ; $81F22A
    phx                         ; $81F22C
    phy                         ; $81F22D
    lda.b $3C                   ; $81F22E
    pha                         ; $81F230
CODE_81F231:
    jsr.w $F252                 ; $81F231
    bcs CODE_81F24B             ; $81F234
    lda.w $FACD,y               ; $81F236
    tay                         ; $81F239
    lda.w $FAB5,y               ; $81F23A
    sec                         ; $81F23D
    sbc.w $FAB5,x               ; $81F23E
    cmp.w #$0060                ; $81F241
    bcs CODE_81F24A             ; $81F244
    dec.b $3C                   ; $81F246
    bne CODE_81F231             ; $81F248
CODE_81F24A:
    clc                         ; $81F24A
CODE_81F24B:
    pla                         ; $81F24B
    sta.b $3C                   ; $81F24C
    ply                         ; $81F24E
    plx                         ; $81F24F
CODE_81F250:
    rts                         ; $81F250
func_81F251:
    rts                         ; $81F251
    lda.w $FADE,y               ; $81F252
    ora.w $FADE,x               ; $81F255
    bit.w #$0088                ; $81F258
    bne CODE_81F282             ; $81F25B
    lda.w $FABF,y               ; $81F25D
    sec                         ; $81F260
    sbc.w $FABF,x               ; $81F261
    bpl CODE_81F26D             ; $81F264
    cmp.w #$FFE8                ; $81F266
    bpl CODE_81F274             ; $81F269
    clc                         ; $81F26B
    rts                         ; $81F26C
CODE_81F26D:
    cmp.w #$0018                ; $81F26D
    bmi CODE_81F274             ; $81F270
    clc                         ; $81F272
    rts                         ; $81F273
CODE_81F274:
    lda.w $FABB,y               ; $81F274
    sec                         ; $81F277
    sbc.w $FABB,x               ; $81F278
    bpl CODE_81F284             ; $81F27B
    cmp.w #$FFC0                ; $81F27D
    bpl CODE_81F28B             ; $81F280
CODE_81F282:
    clc                         ; $81F282
    rts                         ; $81F283
CODE_81F284:
    cmp.w #$0040                ; $81F284
    bmi CODE_81F28B             ; $81F287
    clc                         ; $81F289
    rts                         ; $81F28A
CODE_81F28B:
    phx                         ; $81F28B
    phy                         ; $81F28C
    stx.b $14                   ; $81F28D
    sty.b $16                   ; $81F28F
    lda.w $03DC                 ; $81F291
    and.w #$0003                ; $81F294
    db $D0,$1C ; bne            ; $81F297
    lda.w #$FFFF                ; $81F299
    sta.b $18                   ; $81F29C
    txa                         ; $81F29E
    bne CODE_81F2A6             ; $81F29F
    lda.w #$0000                ; $81F2A1
    sta.b $18                   ; $81F2A4
CODE_81F2A6:
    lda.w #$FFFF                ; $81F2A6
    sta.b $1A                   ; $81F2A9
    tya                         ; $81F2AB
    bne CODE_81F2B3             ; $81F2AC
    lda.w #$0000                ; $81F2AE
    sta.b $1A                   ; $81F2B1
CODE_81F2B3:
    bra CODE_81F2F5             ; $81F2B3
    incbin !baserom:$F2B5..$F2F5
CODE_81F2F5:
    ldx.b $14                   ; $81F2F5
    ldy.b $16                   ; $81F2F7
    lda.w $FAC1,y               ; $81F2F9
    sec                         ; $81F2FC
    sbc.w $FAC1,x               ; $81F2FD
    sta.b $1C                   ; $81F300
    lda.w $FAC3,y               ; $81F302
    sbc.w $FAC3,x               ; $81F305
    sta.b $1E                   ; $81F308
    lda.b $1C                   ; $81F30A
    cmp.w #$0060                ; $81F30C
    lda.b $1E                   ; $81F30F
    sbc.w #$0000                ; $81F311
    bpl CODE_81F368             ; $81F314
    lda.w $FAD5,y               ; $81F316
    cmp.w $FAD5,x               ; $81F319
    bpl CODE_81F343             ; $81F31C
    lda.w $FABB,y               ; $81F31E
    clc                         ; $81F321
    adc.w #$0040                ; $81F322
    sta.w $FABB,x               ; $81F325
    ldx.b $18                   ; $81F328
    bmi CODE_81F33F             ; $81F32A
    pha                         ; $81F32C
    lda.w $0244,x               ; $81F32D
    bit.w #$0004                ; $81F330
    bne CODE_81F33E             ; $81F333
    lda.b $01,s                 ; $81F335
    clc                         ; $81F337
    adc.w $029C,x               ; $81F338
    sta.w $0206,x               ; $81F33B
CODE_81F33E:
    pla                         ; $81F33E
CODE_81F33F:
    ply                         ; $81F33F
    plx                         ; $81F340
    clc                         ; $81F341
    rts                         ; $81F342
CODE_81F343:
    lda.w $FABB,y               ; $81F343
    sec                         ; $81F346
    sbc.w #$0040                ; $81F347
    sta.w $FABB,x               ; $81F34A
    ldx.b $18                   ; $81F34D
    bmi CODE_81F364             ; $81F34F
    pha                         ; $81F351
    lda.w $0244,x               ; $81F352
    bit.w #$0004                ; $81F355
    bne CODE_81F363             ; $81F358
    lda.b $01,s                 ; $81F35A
    clc                         ; $81F35C
    adc.w $029C,x               ; $81F35D
    sta.w $0206,x               ; $81F360
CODE_81F363:
    pla                         ; $81F363
CODE_81F364:
    ply                         ; $81F364
    plx                         ; $81F365
    clc                         ; $81F366
    rts                         ; $81F367
CODE_81F368:
    ldy.b $1A                   ; $81F368
    bmi CODE_81F39E             ; $81F36A
    ldx.b $14                   ; $81F36C
    lda.w $022B,y               ; $81F36E
    sta.b $24                   ; $81F371
    lda.w $FAC9,x               ; $81F373
    asl a                       ; $81F376
    sta.b $20                   ; $81F377
    lda.w $FACB,x               ; $81F379
    rol a                       ; $81F37C
    sta.b $22                   ; $81F37D
    lda.w #$0000                ; $81F37F
    rol a                       ; $81F382
    ldx.w #$0028                ; $81F383
CODE_81F386:
    cmp.b $24                   ; $81F386
    bcc CODE_81F38C             ; $81F388
    sbc.b $24                   ; $81F38A
CODE_81F38C:
    rol.b $20                   ; $81F38C
    rol.b $22                   ; $81F38E
    rol a                       ; $81F390
    dex                         ; $81F391
    bne CODE_81F386             ; $81F392
    lda.b $20                   ; $81F394
    sta.w $0216,y               ; $81F396
    lda.b $22                   ; $81F399
    sta.w $0218,y               ; $81F39B
CODE_81F39E:
    ldx.b $18                   ; $81F39E
    bmi CODE_81F3E5             ; $81F3A0
    lda.w $0244,x               ; $81F3A2
    bit.w #$0004                ; $81F3A5
    bne CODE_81F3B3             ; $81F3A8
    lda.w #$FFFB                ; $81F3AA
    and.w $027C,x               ; $81F3AD
    sta.w $027C,x               ; $81F3B0
CODE_81F3B3:
    ldy.b $16                   ; $81F3B3
    lda.w $022B,x               ; $81F3B5
    sta.b $24                   ; $81F3B8
    lda.w $FAC9,y               ; $81F3BA
    asl a                       ; $81F3BD
    sta.b $20                   ; $81F3BE
    lda.w $FACB,y               ; $81F3C0
    rol a                       ; $81F3C3
    sta.b $22                   ; $81F3C4
    lda.w #$0000                ; $81F3C6
    rol a                       ; $81F3C9
    ldy.w #$0028                ; $81F3CA
CODE_81F3CD:
    cmp.b $24                   ; $81F3CD
    bcc CODE_81F3D3             ; $81F3CF
    sbc.b $24                   ; $81F3D1
CODE_81F3D3:
    rol.b $20                   ; $81F3D3
    rol.b $22                   ; $81F3D5
    rol a                       ; $81F3D7
    dey                         ; $81F3D8
    bne CODE_81F3CD             ; $81F3D9
    lda.b $20                   ; $81F3DB
    sta.w $0216,x               ; $81F3DD
    lda.b $22                   ; $81F3E0
    sta.w $0218,x               ; $81F3E2
CODE_81F3E5:
    ldx.b $14                   ; $81F3E5
    ldy.b $16                   ; $81F3E7
    lda.w $FAC9,x               ; $81F3E9
    pha                         ; $81F3EC
    lda.w $FAC9,y               ; $81F3ED
    sta.w $FAC9,x               ; $81F3F0
    pla                         ; $81F3F3
    sta.w $FAC9,y               ; $81F3F4
    lda.w $FACB,x               ; $81F3F7
    pha                         ; $81F3FA
    lda.w $FACB,y               ; $81F3FB
    sta.w $FACB,x               ; $81F3FE
    pla                         ; $81F401
    sta.w $FACB,y               ; $81F402
    lda.w $FAB5,x               ; $81F405
    sta.b $1C                   ; $81F408
    lda.w $FAB7,x               ; $81F40A
    sta.b $1E                   ; $81F40D
    lda.w $FAB5,y               ; $81F40F
    sec                         ; $81F412
    sbc.w #$0060                ; $81F413
    sta.w $FAB5,x               ; $81F416
    lda.w $FAB7,y               ; $81F419
    sbc.w #$0000                ; $81F41C
    sta.w $FAB7,x               ; $81F41F
    lda.w $FAB5,x               ; $81F422
    cmp.w $FAC1,x               ; $81F425
    lda.w $FAB7,x               ; $81F428
    sbc.w $FAC3,x               ; $81F42B
    bpl CODE_81F43C             ; $81F42E
    incbin !baserom:$F430..$F43C
CODE_81F43C:
    lda.b $1C                   ; $81F43C
    sec                         ; $81F43E
    sbc.w $FAB5,x               ; $81F43F
    sta.b $1C                   ; $81F442
    lda.b $1E                   ; $81F444
    sbc.w $FAB7,x               ; $81F446
    sta.b $1E                   ; $81F449
    ldx.b $18                   ; $81F44B
    bmi CODE_81F468             ; $81F44D
    lda.w $0244,x               ; $81F44F
    bit.w #$0001                ; $81F452
    bne CODE_81F468             ; $81F455
    lda.w $0225,x               ; $81F457
    sec                         ; $81F45A
    sbc.b $1C                   ; $81F45B
    sta.w $0225,x               ; $81F45D
    lda.w $0227,x               ; $81F460
    sbc.b $1E                   ; $81F463
    sta.w $0227,x               ; $81F465
CODE_81F468:
    lda.w #$0004                ; $81F468
    ldx.b $18                   ; $81F46B
    bmi CODE_81F472             ; $81F46D
    and.w $0244,x               ; $81F46F
CODE_81F472:
    ldx.b $1A                   ; $81F472
    bmi CODE_81F479             ; $81F474
    and.w $0244,x               ; $81F476
CODE_81F479:
    and.w #$0004                ; $81F479
    bne CODE_81F487             ; $81F47C
    ldx.w #$8000                ; $81F47E
    lda.w #$FF24                ; $81F481
    jsr.w $F48B                 ; $81F484
CODE_81F487:
    sec                         ; $81F487
    ply                         ; $81F488
    plx                         ; $81F489
    rts                         ; $81F48A
    bit.w $047C                 ; $81F48B
    bmi CODE_81F4AC             ; $81F48E
    pha                         ; $81F490
    lda.w $0223                 ; $81F491
    beq CODE_81F4A2             ; $81F494
    incbin !baserom:$F496..$F4A2
CODE_81F4A2:
    pla                         ; $81F4A2
    stz.w $0466                 ; $81F4A3
    stx.w $0465                 ; $81F4A6
    sta.w $0463                 ; $81F4A9
CODE_81F4AC:
    rts                         ; $81F4AC
    incbin !baserom:$F4AD..$F887
    lda.w #$FFFF                ; $81F887
    sta.w $FAC7,y               ; $81F88A
    lda.w $1ECE                 ; $81F88D
    beq CODE_81F893             ; $81F890
    rts                         ; $81F892
CODE_81F893:
    lda.w #$0000                ; $81F893
    sta.l $308000               ; $81F896
    lda.w $FACB,y               ; $81F89A
    sta.l $308000               ; $81F89D
    lda.w $1E9E                 ; $81F8A1
    sta.l $308000               ; $81F8A4
    lda.w $FAB5,y               ; $81F8A8
    clc                         ; $81F8AB
    adc.l $308000               ; $81F8AC
    sta.w $FAB5,y               ; $81F8B0
    lda.w $FAB7,y               ; $81F8B3
    adc.l $308000               ; $81F8B6
    sta.w $FAB7,y               ; $81F8BA
    jsr.w $FBB1                 ; $81F8BD
    lda.w #$0000                ; $81F8C0
    sta.l $308000               ; $81F8C3
    lda.w $FACB,y               ; $81F8C7
    sta.l $308000               ; $81F8CA
    lda.w $1E9E                 ; $81F8CE
    xba                         ; $81F8D1
    sta.l $308000               ; $81F8D2
    nop                         ; $81F8D6
    lda.l $308000               ; $81F8D7
    sta.b $14                   ; $81F8DB
    lda.l $308000               ; $81F8DD
    sta.b $16                   ; $81F8E1
    lda.w #$0000                ; $81F8E3
    sta.l $308000               ; $81F8E6
    lda.w $FADB,y               ; $81F8EA
    sta.l $308000               ; $81F8ED
    lda.b $15                   ; $81F8F1
    sta.l $308000               ; $81F8F3
    lda.w $FAB9,y               ; $81F8F7
    adc.l $308000               ; $81F8FA
    sta.w $FAB9,y               ; $81F8FE
    lda.w $FABB,y               ; $81F901
    adc.l $308000               ; $81F904
    sta.w $FABB,y               ; $81F908
    lda.w #$000A                ; $81F90B
    sta.l $308000               ; $81F90E
    lda.w $FAB6,y               ; $81F912
    asl a                       ; $81F915
    tax                         ; $81F916
    lda.w $FAB4,y               ; $81F917
    and.w #$FF00                ; $81F91A
    lsr a                       ; $81F91D
    sta.l $308000               ; $81F91E
    nop                         ; $81F922
    lda.l $7ED1B3,x             ; $81F923
    sta.l $308000               ; $81F927
    nop                         ; $81F92B
    nop                         ; $81F92C
    nop                         ; $81F92D
    nop                         ; $81F92E
    nop                         ; $81F92F
    lda.l $7ED1B5,x             ; $81F930
    sta.l $308000               ; $81F934
    nop                         ; $81F938
    nop                         ; $81F939
    nop                         ; $81F93A
    nop                         ; $81F93B
    nop                         ; $81F93C
    lda.l $308000               ; $81F93D
    sta.b $18                   ; $81F941
    nop                         ; $81F943
    lda.l $308000               ; $81F944
    sta.b $1A                   ; $81F948
    nop                         ; $81F94A
    lda.l $308000               ; $81F94B
    sta.b $1C                   ; $81F94F
    nop                         ; $81F951
    lda.l $308000               ; $81F952
    sta.b $1E                   ; $81F956
    lda.w $FABB,y               ; $81F958
    cmp.b $1A                   ; $81F95B
    bmi CODE_81F977             ; $81F95D
    lda.w $FABB,y               ; $81F95F
    sec                         ; $81F962
    sbc.w $1E9E                 ; $81F963
    sta.w $FABB,y               ; $81F966
    lda.w $FADB,y               ; $81F969
    bmi CODE_81F991             ; $81F96C
    eor.w #$FFFF                ; $81F96E
    inc a                       ; $81F971
    sta.w $FADB,y               ; $81F972
    bra CODE_81F991             ; $81F975
CODE_81F977:
    cmp.b $18                   ; $81F977
    bpl CODE_81F991             ; $81F979
    lda.w $FABB,y               ; $81F97B
    clc                         ; $81F97E
    adc.w $1E9E                 ; $81F97F
    sta.w $FABB,y               ; $81F982
    lda.w $FADB,y               ; $81F985
    bpl CODE_81F991             ; $81F988
    eor.w #$FFFF                ; $81F98A
    inc a                       ; $81F98D
    sta.w $FADB,y               ; $81F98E
CODE_81F991:
    lda.w $FAE8,y               ; $81F991
    beq CODE_81F9AB             ; $81F994
    lda.w $1E9E                 ; $81F996
    asl a                       ; $81F999
    asl a                       ; $81F99A
    asl a                       ; $81F99B
    eor.w #$FFFF                ; $81F99C
    sec                         ; $81F99F
    adc.w $FAE8,y               ; $81F9A0
    bpl CODE_81F9A8             ; $81F9A3
    lda.w #$0000                ; $81F9A5
CODE_81F9A8:
    sta.w $FAE8,y               ; $81F9A8
CODE_81F9AB:
    lda.w $FACD,y               ; $81F9AB
    tax                         ; $81F9AE
    lda.w $FAB6,x               ; $81F9AF
    cmp.w $FAB6,y               ; $81F9B2
    beq CODE_81FA06             ; $81F9B5
    lda.w #$0000                ; $81F9B7
    sec                         ; $81F9BA
    sbc.w $FAC9,y               ; $81F9BB
    sta.b $14                   ; $81F9BE
    lda.w $FAE9,y               ; $81F9C0
    and.w #$00FF                ; $81F9C3
    clc                         ; $81F9C6
    adc.w $FAD7,y               ; $81F9C7
    sbc.w $FACB,y               ; $81F9CA
    sta.b $16                   ; $81F9CD
    bmi CODE_81FA44             ; $81F9CF
    bne CODE_81F9DA             ; $81F9D1
    lda.b $14                   ; $81F9D3
    cmp.w $FAD9,y               ; $81F9D5
    bcc CODE_81F9DD             ; $81F9D8
CODE_81F9DA:
    lda.w $FAD9,y               ; $81F9DA
CODE_81F9DD:
    pha                         ; $81F9DD
    lda.w #$0000                ; $81F9DE
    sta.l $308000               ; $81F9E1
    pla                         ; $81F9E5
    sta.l $308000               ; $81F9E6
    lda.w $1E9E                 ; $81F9EA
    sta.l $308000               ; $81F9ED
    lda.w $FAC9,y               ; $81F9F1
    clc                         ; $81F9F4
    adc.l $308000               ; $81F9F5
    sta.w $FAC9,y               ; $81F9F9
    lda.w $FACB,y               ; $81F9FC
    adc.l $308000               ; $81F9FF
    sta.w $FACB,y               ; $81FA03
CODE_81FA06:
    lda.w $FAE6,y               ; $81FA06
    beq CODE_81FA43             ; $81FA09
    pha                         ; $81FA0B
    lda.w #$0000                ; $81FA0C
    sta.l $308000               ; $81FA0F
    pla                         ; $81FA13
    sta.l $308000               ; $81FA14
    lda.w $1E9E                 ; $81FA18
    sta.l $308000               ; $81FA1B
    lda.w $FAC9,y               ; $81FA1F
    clc                         ; $81FA22
    adc.l $308000               ; $81FA23
    sta.w $FAC9,y               ; $81FA27
    lda.w $FACB,y               ; $81FA2A
    adc.l $308000               ; $81FA2D
    sta.w $FACB,y               ; $81FA31
    lda.w $FAE6,y               ; $81FA34
    sec                         ; $81FA37
    sbc.w $1E9E                 ; $81FA38
    bpl CODE_81FA40             ; $81FA3B
    lda.w #$0000                ; $81FA3D
CODE_81FA40:
    sta.w $FAE6,y               ; $81FA40
CODE_81FA43:
    rts                         ; $81FA43
CODE_81FA44:
    lda.b $14                   ; $81FA44
    eor.w #$FFFF                ; $81FA46
    sec                         ; $81FA49
    adc.w #$0000                ; $81FA4A
    sta.b $14                   ; $81FA4D
    lda.b $16                   ; $81FA4F
    eor.w #$FFFF                ; $81FA51
    adc.w #$0000                ; $81FA54
    sta.b $16                   ; $81FA57
    bne CODE_81FA62             ; $81FA59
    lda.b $14                   ; $81FA5B
    cmp.w $FAD9,y               ; $81FA5D
    bcc CODE_81FA65             ; $81FA60
CODE_81FA62:
    lda.w $FAD9,y               ; $81FA62
CODE_81FA65:
    pha                         ; $81FA65
    lda.w #$0000                ; $81FA66
    sta.l $308000               ; $81FA69
    pla                         ; $81FA6D
    sta.l $308000               ; $81FA6E
    lda.w $1E9E                 ; $81FA72
    sta.l $308000               ; $81FA75
    lda.w $FAC9,y               ; $81FA79
    sec                         ; $81FA7C
    sbc.l $308000               ; $81FA7D
    sta.w $FAC9,y               ; $81FA81
    lda.w $FACB,y               ; $81FA84
    sbc.l $308000               ; $81FA87
    sta.w $FACB,y               ; $81FA8B
    rts                         ; $81FA8E
    lda.w $0244                 ; $81FA8F
    bit.w #$0001                ; $81FA92
    bne CODE_81FAE0             ; $81FA95
    lda.w $FAB5,y               ; $81FA97
    clc                         ; $81FA9A
    adc.w $022D                 ; $81FA9B
    sta.w $FAB5,y               ; $81FA9E
    lda.w $FAB7,y               ; $81FAA1
    adc.w $022F                 ; $81FAA4
    sta.w $FAB7,y               ; $81FAA7
    lda.w $020E                 ; $81FAAA
    sta.w $FAC9,y               ; $81FAAD
    lda.w $0210                 ; $81FAB0
    sta.w $FACB,y               ; $81FAB3
    lda.w $0204                 ; $81FAB6
    sta.w $FAB9,y               ; $81FAB9
    lda.w $0206                 ; $81FABC
    sta.w $FABB,y               ; $81FABF
    lda.w #$FFFF                ; $81FAC2
    sta.w $FAC7,y               ; $81FAC5
    jsr.w $FBB1                 ; $81FAC8
    lda.w $FADE,y               ; $81FACB
    and.w #$FFDF                ; $81FACE
    bit.w $1E66                 ; $81FAD1
    bpl CODE_81FAD9             ; $81FAD4
    incbin !baserom:$FAD6..$FAD9
CODE_81FAD9:
    ora.w #$0010                ; $81FAD9
    sta.w $FADE,y               ; $81FADC
    rts                         ; $81FADF
CODE_81FAE0:
    lda.w #$0000                ; $81FAE0
    sta.l $308000               ; $81FAE3
    lda.w $1E9E                 ; $81FAE7
    sta.l $308000               ; $81FAEA
    lda.w $0210                 ; $81FAEE
    sta.l $308000               ; $81FAF1
    lda.w $FAB5,y               ; $81FAF5
    clc                         ; $81FAF8
    adc.l $308000               ; $81FAF9
    sta.w $FAB5,y               ; $81FAFD
    lda.w $FAB7,y               ; $81FB00
    adc.l $308000               ; $81FB03
    sta.w $FAB7,y               ; $81FB07
    lda.w #$FFFF                ; $81FB0A
    sta.w $FAC7,y               ; $81FB0D
    lda.w $020E                 ; $81FB10
    sta.w $FAC9,y               ; $81FB13
    lda.w $0210                 ; $81FB16
    sta.w $FACB,y               ; $81FB19
    jsr.w $FBB1                 ; $81FB1C
    rts                         ; $81FB1F
    incbin !baserom:$FB20..$FBB1
    lda.w $FADE,y               ; $81FBB1
    bit.w #$0040                ; $81FBB4
    bne CODE_81FBDE             ; $81FBB7
    ora.w #$0040                ; $81FBB9
    sta.w $FADE,y               ; $81FBBC
CODE_81FBBF:
    lda.w $FAE0,y               ; $81FBBF
    tax                         ; $81FBC2
    lda.w $FAB6,y               ; $81FBC3
    sec                         ; $81FBC6
    sbc.l $840000,x             ; $81FBC7
    bcc CODE_81FBDD             ; $81FBCB
    cmp.w #$003F                ; $81FBCD
    bcc CODE_81FBDD             ; $81FBD0
    txa                         ; $81FBD2
    clc                         ; $81FBD3
    adc.w #$0014                ; $81FBD4
    sta.w $FAE0,y               ; $81FBD7
    tax                         ; $81FBDA
    bra CODE_81FBBF             ; $81FBDB
CODE_81FBDD:
    rts                         ; $81FBDD
CODE_81FBDE:
    lda.w $FAE0,y               ; $81FBDE
    tax                         ; $81FBE1
    lda.w $FAB6,y               ; $81FBE2
    sec                         ; $81FBE5
    sbc.l $840000,x             ; $81FBE6
    bcc CODE_81FC3A             ; $81FBEA
    pha                         ; $81FBEC
    lda.w $FADE,y               ; $81FBED
    bit.w #$0010                ; $81FBF0
    bne CODE_81FC0E             ; $81FBF3
    lda.w $FADE,y               ; $81FBF5
    and.w #$FFDF                ; $81FBF8
    sta.w $FADE,y               ; $81FBFB
    jsl.l rng_next_long         ; $81FBFE
    and.w #$0020                ; $81FC02
    ora.w $FADE,y               ; $81FC05
    ora.w #$0010                ; $81FC08
    sta.w $FADE,y               ; $81FC0B
CODE_81FC0E:
    pla                         ; $81FC0E
    cmp.w #$003F                ; $81FC0F
    bcc CODE_81FC3A             ; $81FC12
    txa                         ; $81FC14
    clc                         ; $81FC15
    adc.w #$0014                ; $81FC16
    sta.w $FAE0,y               ; $81FC19
    lda.w $FADE,y               ; $81FC1C
    bit.w #$0020                ; $81FC1F
    bne CODE_81FC59             ; $81FC22
    lda.l $840010,x             ; $81FC24
    clc                         ; $81FC28
    adc.w $FAB6,y               ; $81FC29
    sta.w $FAB6,y               ; $81FC2C
    lda.w #$0080                ; $81FC2F
    ora.w $FADE,y               ; $81FC32
    sta.w $FADE,y               ; $81FC35
    bra CODE_81FBDE             ; $81FC38
CODE_81FC3A:
    lda.w $FAB6,y               ; $81FC3A
    cmp.w $FAE2,y               ; $81FC3D
    bcc CODE_81FC58             ; $81FC40
    clc                         ; $81FC42
    adc.w $FAE4,y               ; $81FC43
    sta.w $FAB6,y               ; $81FC46
    lda.w #$7000                ; $81FC49
    sta.w $FAE2,y               ; $81FC4C
    lda.w #$0080                ; $81FC4F
    ora.w $FADE,y               ; $81FC52
    sta.w $FADE,y               ; $81FC55
CODE_81FC58:
    rts                         ; $81FC58
CODE_81FC59:
    lda.l $840000,x             ; $81FC59
    clc                         ; $81FC5D
    adc.l $840010,x             ; $81FC5E
    adc.w #$003F                ; $81FC62
    sta.w $FAE2,y               ; $81FC65
    lda.l $840012,x             ; $81FC68
    sta.w $FAE4,y               ; $81FC6C
    jmp.w $FBDE                 ; $81FC6F
    incbin !baserom:$FC72..$10000
