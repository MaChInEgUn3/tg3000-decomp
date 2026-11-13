; bank $8A (file offset $050000)
org $8A8000

    rtl                         ; $8A8000
    ldx.w #$007E                ; $8A8001
CODE_8A8004:
    stz.b $14,x                 ; $8A8004
    dex                         ; $8A8006
    dex                         ; $8A8007
    bpl CODE_8A8004             ; $8A8008
    rtl                         ; $8A800A
    stz.w $0196                 ; $8A800B
    stz.w $0198                 ; $8A800E
    stz.w $019A                 ; $8A8011
    sep.b #$20                  ; $8A8014
    stz.w $420C                 ; $8A8016
    stz.w $420B                 ; $8A8019
    rep.b #$20                  ; $8A801C
    ldx.w #$9307                ; $8A801E
    lda.w #$008A                ; $8A8021
    jsl.l $8081C8               ; $8A8024
    lda.w #$FBD1                ; $8A8028
    sta.w $0008                 ; $8A802B
    jsl.l $8A8001               ; $8A802E
    jsl.l $8A8DEC               ; $8A8032
    stz.b $8C                   ; $8A8036
    ldx.w #$001E                ; $8A8038
CODE_8A803B:
    stz.w $1516,x               ; $8A803B
    dex                         ; $8A803E
    dex                         ; $8A803F
    bpl CODE_8A803B             ; $8A8040
    lda.w #$0001                ; $8A8042
    jsl.l $8A8BB6               ; $8A8045
    jsr.w $8768                 ; $8A8049
    jsl.l $8A8F18               ; $8A804C
    jsl.l $8A8001               ; $8A8050
    jsl.l $8A8EEF               ; $8A8054
    jsr.w $8828                 ; $8A8058
    jsr.w $891C                 ; $8A805B
    jsr.w $8B0A                 ; $8A805E
    jsl.l $8A8E1B               ; $8A8061
    jsr.w $8787                 ; $8A8065
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
    jsr.w $82C0                 ; $8A8085
    jsr.w $8232                 ; $8A8088
    jsr.w $823D                 ; $8A808B
    jsr.w $8290                 ; $8A808E
    jsr.w $824B                 ; $8A8091
    jsr.w $867C                 ; $8A8094
    jsr.w $869A                 ; $8A8097
    jsr.w $86AB                 ; $8A809A
    jsr.w $898F                 ; $8A809D
    jsl.l $8A8E1B               ; $8A80A0
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
    jsr.w $81BA                 ; $8A80C5
    lda.w #$0001                ; $8A80C8
    jsl.l $8A8BB6               ; $8A80CB
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
    jsr.w $8181                 ; $8A80E7
    lda.w #$0001                ; $8A80EA
    jsl.l $8A8BB6               ; $8A80ED
    jsr.w $8B0A                 ; $8A80F1
    lda.b $5A                   ; $8A80F4
    beq CODE_8A8142             ; $8A80F6
    incbin !baserom:$500F8..$50142
CODE_8A8142:
    jsr.w $85AF                 ; $8A8142
    lda.w #$0002                ; $8A8145
    tsb.w $0026                 ; $8A8148
    jsr.w $82C0                 ; $8A814B
    jsr.w $8232                 ; $8A814E
    jsr.w $823D                 ; $8A8151
    jsr.w $8290                 ; $8A8154
    jsr.w $824B                 ; $8A8157
    jsr.w $867C                 ; $8A815A
    jsr.w $86AB                 ; $8A815D
    jsr.w $898F                 ; $8A8160
    jsr.w $869A                 ; $8A8163
    jsl.l $8A8E1B               ; $8A8166
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
    jsl.l $8A8BB6               ; $8A817C
    rtl                         ; $8A8180
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
    lda.w #$7E80                ; $8A81BA
    ldx.w #$81E2                ; $8A81BD
    ldy.w #$0020                ; $8A81C0
    jsl.l $8083CB               ; $8A81C3
    lda.w #$7E90                ; $8A81C7
    ldx.w #$8202                ; $8A81CA
    ldy.w #$0020                ; $8A81CD
    jsl.l $8083CB               ; $8A81D0
    lda.w #$7EA0                ; $8A81D4
    ldx.w #$8222                ; $8A81D7
    ldy.w #$0020                ; $8A81DA
    jsl.l $8083CB               ; $8A81DD
    lda.w #$7E01                ; $8A81E1
    ldx.w #$8114                ; $8A81E4
    ldy.w #$001E                ; $8A81E7
    jsl.l $8083CB               ; $8A81EA
    lda.w $1536                 ; $8A81EE
    asl a                       ; $8A81F1
    asl a                       ; $8A81F2
    asl a                       ; $8A81F3
    tax                         ; $8A81F4
    lda.l $8A934F,x             ; $8A81F5
    tax                         ; $8A81F9
    lda.w #$7EA0                ; $8A81FA
    ldy.w #$0020                ; $8A81FD
    jsl.l $8083CB               ; $8A8200
    rts                         ; $8A8204
    incbin !baserom:$50205..$50232
    lda.b $36                   ; $8A8232
    lsr a                       ; $8A8234
    lsr a                       ; $8A8235
    clc                         ; $8A8236
    adc.w #$0108                ; $8A8237
    sta.b $4A                   ; $8A823A
    rts                         ; $8A823C
    lda.b $38                   ; $8A823D
    lsr a                       ; $8A823F
    lsr a                       ; $8A8240
    clc                         ; $8A8241
    adc.w #$FFB7                ; $8A8242
    clc                         ; $8A8245
    adc.b $64                   ; $8A8246
    sta.b $4C                   ; $8A8248
    rts                         ; $8A824A
    lda.b $62                   ; $8A824B
    cmp.w #$FFFF                ; $8A824D
    beq CODE_8A8253             ; $8A8250
    incbin !baserom:$50252..$50253
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
    lda.b $54                   ; $8A8290
    db $D0,$01 ; bne            ; $8A8292
    rts                         ; $8A8294
    incbin !baserom:$50295..$502C0
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
    jsr.w $8325                 ; $8A82E8
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
    jsr.w $8325                 ; $8A830B
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
    incbin !baserom:$50322..$50325
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
    incbin !baserom:$5033C..$505AF
    lda.b $42                   ; $8A85AF
    beq CODE_8A85CB             ; $8A85B1
    incbin !baserom:$505B3..$505CB
CODE_8A85CB:
    rts                         ; $8A85CB
    incbin !baserom:$505CC..$5067C
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
    jsr.w $9202                 ; $8A86E9
    clc                         ; $8A86EC
    lda.b $84                   ; $8A86ED
    adc.w #$0020                ; $8A86EF
    sta.b $7C                   ; $8A86F2
    lda.b $7E                   ; $8A86F4
    adc.w #$00C8                ; $8A86F6
    sta.b $7A                   ; $8A86F9
    jsr.w $9202                 ; $8A86FB
    clc                         ; $8A86FE
    lda.b $84                   ; $8A86FF
    sta.b $7C                   ; $8A8701
    lda.b $80                   ; $8A8703
    adc.w #$000A                ; $8A8705
    sta.b $7A                   ; $8A8708
    jsr.w $92CF                 ; $8A870A
    clc                         ; $8A870D
    lda.b $84                   ; $8A870E
    adc.w #$0032                ; $8A8710
    sta.b $7C                   ; $8A8713
    lda.b $82                   ; $8A8715
    adc.w #$0012                ; $8A8717
    sta.b $7A                   ; $8A871A
    jsr.w $92CF                 ; $8A871C
    clc                         ; $8A871F
    lda.b $84                   ; $8A8720
    adc.w #$0028                ; $8A8722
    sta.b $7C                   ; $8A8725
    lda.b $80                   ; $8A8727
    adc.w #$00F0                ; $8A8729
    sta.b $7A                   ; $8A872C
    jsr.w $92CF                 ; $8A872E
    clc                         ; $8A8731
    lda.b $84                   ; $8A8732
    adc.w #$0032                ; $8A8734
    sta.b $7C                   ; $8A8737
    lda.b $82                   ; $8A8739
    adc.w #$00C8                ; $8A873B
    sta.b $7A                   ; $8A873E
    jsr.w $92CF                 ; $8A8740
    clc                         ; $8A8743
    lda.b $84                   ; $8A8744
    adc.w #$0019                ; $8A8746
    sta.b $7C                   ; $8A8749
    lda.b $82                   ; $8A874B
    adc.w #$0080                ; $8A874D
    sta.b $7A                   ; $8A8750
    jsr.w $92CF                 ; $8A8752
    clc                         ; $8A8755
    lda.b $84                   ; $8A8756
    sbc.w #$000A                ; $8A8758
    sta.b $7C                   ; $8A875B
    lda.b $80                   ; $8A875D
    adc.w #$0140                ; $8A875F
    sta.b $7A                   ; $8A8762
    jsr.w $92CF                 ; $8A8764
    rts                         ; $8A8767
    lda.w $1536                 ; $8A8768
    asl a                       ; $8A876B
    asl a                       ; $8A876C
    asl a                       ; $8A876D
    tax                         ; $8A876E
    lda.l $8A9351,x             ; $8A876F
    and.w #$0002                ; $8A8773
    bne CODE_8A8779             ; $8A8776
    incbin !baserom:$50778..$50779
CODE_8A8779:
    phb                         ; $8A8779
    pea.w $8383                 ; $8A877A
    plb                         ; $8A877D
    plb                         ; $8A877E
    lda.w $1536                 ; $8A877F
    jsr.w $8BC2                 ; $8A8782
    plb                         ; $8A8785
    rts                         ; $8A8786
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
    lda.w #$007E                ; $8A891C
    ldx.w #$8256                ; $8A891F
    ldy.w #$2000                ; $8A8922
    jsl.l $80837D               ; $8A8925
    lda.w #$007E                ; $8A8929
    ldx.w #$B385                ; $8A892C
    ldy.w #$2A00                ; $8A892F
    jsl.l $80837D               ; $8A8932
    lda.w #$007E                ; $8A8936
    ldx.w #$9F07                ; $8A8939
    ldy.w #$3000                ; $8A893C
    jsl.l $80837D               ; $8A893F
    lda.w #$007E                ; $8A8943
    sta.b $14                   ; $8A8946
    ldx.w #$A071                ; $8A8948
    ldy.w #$3600                ; $8A894B
    lda.w #$0800                ; $8A894E
    jsl.l $8083AD               ; $8A8951
    stz.b $5A                   ; $8A8955
    ldx.w #$0000                ; $8A8957
    ldy.w #$0200                ; $8A895A
    stx.b $3C                   ; $8A895D
    sty.b $3E                   ; $8A895F
    jsr.w $898F                 ; $8A8961
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
    lda.b $5E                   ; $8A898F
    beq CODE_8A8994             ; $8A8991
    incbin !baserom:$50993..$50994
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
    incbin !baserom:$509A4..$509AD
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
    jsr.w $8AE7                 ; $8A8A49
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
    db $D0,$19 ; bne            ; $8A8B82
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
    incbin !baserom:$50B9D..$50BB6
    clc                         ; $8A8BB6
    adc.l $810024               ; $8A8BB7
CODE_8A8BBB:
    cmp.l $810024               ; $8A8BBB
    bne CODE_8A8BBB             ; $8A8BBF
    rtl                         ; $8A8BC1
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
    jsl.l $82FB70               ; $8A8BDE
    plx                         ; $8A8BE2
    phx                         ; $8A8BE3
    lda.l $810444               ; $8A8BE4
    asl a                       ; $8A8BE8
    tax                         ; $8A8BE9
    jmp.w ($8BED,x)             ; $8A8BEA
    incbin !baserom:$50BED..$50C5B
    plx                         ; $8A8C5B
    lda.w #$0420                ; $8A8C5C
    jsr.w $8CB7                 ; $8A8C5F
    lda.w #$0000                ; $8A8C62
    jsr.w $8D81                 ; $8A8C65
    lda.w #$0200                ; $8A8C68
    jsr.w $8DA0                 ; $8A8C6B
    jsr.w $8DBF                 ; $8A8C6E
    rts                         ; $8A8C71
    plx                         ; $8A8C72
    lda.w #$0820                ; $8A8C73
    jsr.w $8CB7                 ; $8A8C76
    lda.w #$0000                ; $8A8C79
    jsr.w $8D81                 ; $8A8C7C
    lda.w #$0200                ; $8A8C7F
    jsr.w $8DA0                 ; $8A8C82
    jsr.w $8DBF                 ; $8A8C85
    rts                         ; $8A8C88
    incbin !baserom:$50C89..$50CB7
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
    jsl.l $8083AD               ; $8A8D7B
    plx                         ; $8A8D7F
    rts                         ; $8A8D80
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
    jsl.l $8083AD               ; $8A8D9A
    plx                         ; $8A8D9E
    rts                         ; $8A8D9F
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
    jsl.l $8083AD               ; $8A8DB9
    plx                         ; $8A8DBD
    rts                         ; $8A8DBE
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
    jsl.l $8083CB               ; $8A8DE6
    plx                         ; $8A8DEA
    rts                         ; $8A8DEB
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
    rep.b #$20                  ; $8A8EEF
    stz.b $30                   ; $8A8EF1
    ldx.w #$00FE                ; $8A8EF3
CODE_8A8EF6:
    stz.w $0500,x               ; $8A8EF6
    dex                         ; $8A8EF9
    dex                         ; $8A8EFA
    bpl CODE_8A8EF6             ; $8A8EFB
    jsl.l $8A8F02               ; $8A8EFD
    rtl                         ; $8A8F01
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
    jsl.l $8A8FA4               ; $8A8F5F
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
    jsl.l $8A8FA4               ; $8A8F7A
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
    jsl.l $8A8FA4               ; $8A8F9F
    rtl                         ; $8A8FA3
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
    incbin !baserom:$5101F..$51022
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
    lda.w #$00FF                ; $8A94C3
    trb.w $1B38                 ; $8A94C6
    jsl.l $8A9BF6               ; $8A94C9
    jsl.l $8A98B4               ; $8A94CD
    jsl.l $8A9908               ; $8A94D1
    jsl.l $8A8DEC               ; $8A94D5
    stz.w $0196                 ; $8A94D9
    stz.w $0198                 ; $8A94DC
    stz.w $019A                 ; $8A94DF
    ldx.w #$9D31                ; $8A94E2
    lda.w #$008A                ; $8A94E5
    jsl.l $8081C8               ; $8A94E8
    lda.w #$8934                ; $8A94EC
    sta.w $0008                 ; $8A94EF
    ldx.w #$8601                ; $8A94F2
    ldy.w #$1000                ; $8A94F5
    lda.w #$0088                ; $8A94F8
    jsl.l $80837D               ; $8A94FB
    lda.w #$8660                ; $8A94FF
    ldx.w #$F40B                ; $8A9502
    ldy.w #$0040                ; $8A9505
    jsl.l $8083CB               ; $8A9508
    lda.w #$8A10                ; $8A950C
    ldx.w #$9D2D                ; $8A950F
    ldy.w #$0020                ; $8A9512
    jsl.l $8083CB               ; $8A9515
    ldx.w #$AADF                ; $8A9519
    ldy.w #$7400                ; $8A951C
    lda.w #$0091                ; $8A951F
    jsl.l $80837D               ; $8A9522
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
    jsl.l $80FBD8               ; $8A9554
    phb                         ; $8A9558
    lda.w #$8C80                ; $8A9559
    ldx.w #$95C3                ; $8A955C
    ldy.w #$0020                ; $8A955F
    jsl.l $8083CB               ; $8A9562
    lda.w #$8600                ; $8A9566
    ldx.w #$F44B                ; $8A9569
    ldy.w #$0020                ; $8A956C
    jsl.l $8083CB               ; $8A956F
    ldy.w #$0800                ; $8A9573
    ldx.w #$F861                ; $8A9576
    lda.w #$0087                ; $8A9579
    jsl.l $80837D               ; $8A957C
    plb                         ; $8A9580
    jsr.w $B981                 ; $8A9581
    inc.b $6A                   ; $8A9584
    jsr.w $971B                 ; $8A9586
    lda.w #$0000                ; $8A9589
    ldx.w #$0040                ; $8A958C
    jsr.w $9773                 ; $8A958F
    jsl.l $8180D0               ; $8A9592
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
    jsr.w $9C69                 ; $8A95B7
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
    jsr.w $C15D                 ; $8A95E5
    jsr.w $BA7D                 ; $8A95E8
    lda.w #$0001                ; $8A95EB
    jsl.l $8A9880               ; $8A95EE
    lda.w #$0004                ; $8A95F2
    sta.w $420C                 ; $8A95F5
    lda.w #$0F00                ; $8A95F8
    sta.w $0198                 ; $8A95FB
    lda.w #$0080                ; $8A95FE
    sta.w $0196                 ; $8A9601
    stz.b $6E                   ; $8A9604
    lda.w #$0001                ; $8A9606
    jsl.l $8A9880               ; $8A9609
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
    jsr.w $96C2                 ; $8A9637
    lda.b $86                   ; $8A963A
    and.w #$FF00                ; $8A963C
    xba                         ; $8A963F
    tax                         ; $8A9640
    lda.b $7C                   ; $8A9641
    jsr.w $9773                 ; $8A9643
    jsl.l $8180D0               ; $8A9646
    lda.w #$0002                ; $8A964A
    tsb.w $0026                 ; $8A964D
    bit.w $1B37                 ; $8A9650
    bpl CODE_8A965E             ; $8A9653
    incbin !baserom:$51655..$5165E
CODE_8A965E:
    jsr.w $9675                 ; $8A965E
    lda.w #$0001                ; $8A9661
    trb.b $F8                   ; $8A9664
    beq CODE_8A9668             ; $8A9666
CODE_8A9668:
    lda.l $81019A               ; $8A9668
    beq CODE_8A9674             ; $8A966C
    jsr.w $9C48                 ; $8A966E
    jmp.w $9606                 ; $8A9671
CODE_8A9674:
    rtl                         ; $8A9674
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
    jsl.l $8A987F               ; $8A96A0
    plb                         ; $8A96A4
    lda.w #$0000                ; $8A96A5
    tcd                         ; $8A96A8
    jml.l $80FC18               ; $8A96A9
    incbin !baserom:$516AD..$516C2
    inc.b $40                   ; $8A96C2
    lda.b $40                   ; $8A96C4
    clc                         ; $8A96C6
    rts                         ; $8A96C7
    incbin !baserom:$516C8..$5171B
    ldx.w #$0000                ; $8A971B
    lda.w #$2000                ; $8A971E
    sta.b $14                   ; $8A9721
CODE_8A9723:
    jsr.w $988C                 ; $8A9723
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
    jsr.w $988C                 ; $8A973F
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
    jsr.w $9863                 ; $8A9848
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
    jsr.w $988C                 ; $8A9863
    cmp.w #$8000                ; $8A9866
    ror a                       ; $8A9869
    sta.w $0F00,y               ; $8A986A
    jsr.w $988C                 ; $8A986D
    xba                         ; $8A9870
    cmp.w #$8000                ; $8A9871
    ror a                       ; $8A9874
    sta.w $0F02,y               ; $8A9875
    lda.w #$7F00                ; $8A9878
    sta.w $0F04,y               ; $8A987B
    rts                         ; $8A987E
    rtl                         ; $8A987F
    clc                         ; $8A9880
    adc.l $810024               ; $8A9881
CODE_8A9885:
    cmp.l $810024               ; $8A9885
    bne CODE_8A9885             ; $8A9889
    rtl                         ; $8A988B
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
    jsr.w $98F0                 ; $8A98B4
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
    incbin !baserom:$518D3..$518DF
CODE_8A98DF:
    clc                         ; $8A98DF
    lda.w $041C,y               ; $8A98E0
    adc.w $9D8D,x               ; $8A98E3
    sta.w $041C,y               ; $8A98E6
    plx                         ; $8A98E9
    inx                         ; $8A98EA
    dec.b $5A                   ; $8A98EB
    bne CODE_8A98BA             ; $8A98ED
    rtl                         ; $8A98EF
    lda.l $8103DC               ; $8A98F0
    and.w #$000F                ; $8A98F4
    cmp.w #$0002                ; $8A98F7
    bne CODE_8A9902             ; $8A98FA
    incbin !baserom:$518FC..$51902
CODE_8A9902:
    lda.w #$000A                ; $8A9902
    sta.b $5A                   ; $8A9905
    rts                         ; $8A9907
    jsr.w $9AA5                 ; $8A9908
    jsr.w $9BE2                 ; $8A990B
    stz.b $5E                   ; $8A990E
    jsr.w $99BE                 ; $8A9910
CODE_8A9913:
    jsr.w $9BB5                 ; $8A9913
    bcs CODE_8A993E             ; $8A9916
    inc.b $5E                   ; $8A9918
    jsr.w $9A81                 ; $8A991A
    jsr.w $9A99                 ; $8A991D
    jsr.w $99E2                 ; $8A9920
    lda.w #$0016                ; $8A9923
    jsr.w $99B4                 ; $8A9926
    jsr.w $9A99                 ; $8A9929
    jsr.w $9A99                 ; $8A992C
    jsr.w $9A99                 ; $8A992F
    lda.l $81041C,x             ; $8A9932
    jsr.w $9A33                 ; $8A9936
    jsr.w $99D6                 ; $8A9939
    bra CODE_8A9913             ; $8A993C
CODE_8A993E:
    bit.w $1B37                 ; $8A993E
    db $30,$03 ; bmi            ; $8A9941
    jmp.w $999F                 ; $8A9943
    incbin !baserom:$51946..$5199F
    sep.b #$20                  ; $8A999F
    ldx.w #$0000                ; $8A99A1
CODE_8A99A4:
    lda.w $9DBD,x               ; $8A99A4
    cmp.b #$FF                  ; $8A99A7
    beq CODE_8A99B1             ; $8A99A9
    jsr.w $9C19                 ; $8A99AB
    inx                         ; $8A99AE
    bra CODE_8A99A4             ; $8A99AF
CODE_8A99B1:
    rep.b #$20                  ; $8A99B1
    rtl                         ; $8A99B3
CODE_8A99B4:
    cmp.b $60                   ; $8A99B4
    beq CODE_8A99BD             ; $8A99B6
    jsr.w $9A99                 ; $8A99B8
    bra CODE_8A99B4             ; $8A99BB
CODE_8A99BD:
    rts                         ; $8A99BD
    lda.l $8103DC               ; $8A99BE
    and.w #$000F                ; $8A99C2
    cmp.w #$0002                ; $8A99C5
    bne CODE_8A99D0             ; $8A99C8
    incbin !baserom:$519CA..$519D0
CODE_8A99D0:
    lda.w #$0028                ; $8A99D0
    sta.b $66                   ; $8A99D3
    rts                         ; $8A99D5
    sep.b #$20                  ; $8A99D6
    lda.b #$80                  ; $8A99D8
    jsr.w $9C19                 ; $8A99DA
    rep.b #$20                  ; $8A99DD
    stz.b $60                   ; $8A99DF
    rts                         ; $8A99E1
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
    jsr.w $9C19                 ; $8A99F2
    inx                         ; $8A99F5
    bra CODE_8A99EC             ; $8A99F6
CODE_8A99F8:
    rep.b #$20                  ; $8A99F8
    plx                         ; $8A99FA
    rts                         ; $8A99FB
    incbin !baserom:$519FC..$51A33
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
    jsr.w $9C19                 ; $8A9A62
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
    jsr.w $9C19                 ; $8A9A78
    rep.b #$20                  ; $8A9A7B
    ply                         ; $8A9A7D
    plx                         ; $8A9A7E
    pla                         ; $8A9A7F
    rts                         ; $8A9A80
    lda.b $5E                   ; $8A9A81
    cmp.w #$000A                ; $8A9A83
    bpl CODE_8A9A93             ; $8A9A86
    pha                         ; $8A9A88
    sep.b #$20                  ; $8A9A89
    lda.b #$20                  ; $8A9A8B
    jsr.w $9C19                 ; $8A9A8D
    rep.b #$20                  ; $8A9A90
    pla                         ; $8A9A92
CODE_8A9A93:
    lda.b $5E                   ; $8A9A93
    jsr.w $9A33                 ; $8A9A95
    rts                         ; $8A9A98
    pha                         ; $8A9A99
    sep.b #$20                  ; $8A9A9A
    lda.b #$20                  ; $8A9A9C
    jsr.w $9C19                 ; $8A9A9E
    rep.b #$20                  ; $8A9AA1
    pla                         ; $8A9AA3
    rts                         ; $8A9AA4
    stz.b $54                   ; $8A9AA5
    jsr.w $9AB1                 ; $8A9AA7
    jsr.w $9AF1                 ; $8A9AAA
    jsr.w $9AC9                 ; $8A9AAD
    rts                         ; $8A9AB0
    lda.l $8103DC               ; $8A9AB1
    and.w #$000F                ; $8A9AB5
    cmp.w #$0002                ; $8A9AB8
    db $F0,$06 ; beq            ; $8A9ABB
    lda.w #$0014                ; $8A9ABD
    sta.b $56                   ; $8A9AC0
    rts                         ; $8A9AC2
    incbin !baserom:$51AC3..$51AC9
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
    lda.l $8103DC               ; $8A9AF1
    and.w #$000F                ; $8A9AF5
    cmp.w #$0000                ; $8A9AF8
    beq CODE_8A9B2D             ; $8A9AFB
    incbin !baserom:$51AFD..$51B2D
CODE_8A9B2D:
    ldx.w #$037E                ; $8A9B2D
    ldy.w #$0081                ; $8A9B30
    jsr.w $9B85                 ; $8A9B33
    rts                         ; $8A9B36
    incbin !baserom:$51B37..$51B85
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
    jsr.w $9BAA                 ; $8A9BA5
    pla                         ; $8A9BA8
    rts                         ; $8A9BA9
    clc                         ; $8A9BAA
    lda.b $54                   ; $8A9BAB
    adc.w #$0010                ; $8A9BAD
    sta.b $54                   ; $8A9BB0
    dec.b $56                   ; $8A9BB2
    rts                         ; $8A9BB4
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
    phx                         ; $8A9C19
    ldx.b $58                   ; $8A9C1A
    sta.l $7F7D00,x             ; $8A9C1C
    inx                         ; $8A9C20
    stx.b $58                   ; $8A9C21
    plx                         ; $8A9C23
    inc.b $60                   ; $8A9C24
    rts                         ; $8A9C26
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
    jsr.w $9C27                 ; $8A9C48
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
    jsr.w $9C86                 ; $8A9C69
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
    jsr.w $9CF8                 ; $8A9CB0
    lda.l $8103DC               ; $8A9CB3
    bit.w #$000F                ; $8A9CB7
    beq CODE_8A9CC6             ; $8A9CBA
    incbin !baserom:$51CBC..$51CC6
CODE_8A9CC6:
    ldx.w #$0000                ; $8A9CC6
    lda.w $0D00,x               ; $8A9CC9
    beq CODE_8A9CF3             ; $8A9CCC
    incbin !baserom:$51CCE..$51CF3
CODE_8A9CF3:
    jsr.w $9D1E                 ; $8A9CF3
    clc                         ; $8A9CF6
    rtl                         ; $8A9CF7
    ldx.w #$0026                ; $8A9CF8
    lda.w #$0000                ; $8A9CFB
CODE_8A9CFE:
    sta.l $7F7FBC,x             ; $8A9CFE
    dex                         ; $8A9D02
    dex                         ; $8A9D03
    bpl CODE_8A9CFE             ; $8A9D04
    jsr.w $9BE2                 ; $8A9D06
    jsr.w $99D0                 ; $8A9D09
    ldy.w #$0000                ; $8A9D0C
CODE_8A9D0F:
    jsr.w $9BB5                 ; $8A9D0F
    bcs CODE_8A9D1D             ; $8A9D12
    txa                         ; $8A9D14
    lsr a                       ; $8A9D15
    sta.w $0D00,y               ; $8A9D16
    iny                         ; $8A9D19
    iny                         ; $8A9D1A
    bra CODE_8A9D0F             ; $8A9D1B
CODE_8A9D1D:
    rts                         ; $8A9D1D
    ldx.w #$0026                ; $8A9D1E
    lda.w #$0000                ; $8A9D21
CODE_8A9D24:
    sta.l $81041C,x             ; $8A9D24
    dex                         ; $8A9D28
    dex                         ; $8A9D29
    bpl CODE_8A9D24             ; $8A9D2A
    rts                         ; $8A9D2C
    incbin !baserom:$51D2D..$51E5B
    phx                         ; $8A9E5B
    ldx.w #$00FE                ; $8A9E5C
CODE_8A9E5F:
    stz.w $0D00,x               ; $8A9E5F
    dex                         ; $8A9E62
    dex                         ; $8A9E63
    bpl CODE_8A9E5F             ; $8A9E64
    plx                         ; $8A9E66
    cpx.w #$0000                ; $8A9E67
    beq CODE_8A9E82             ; $8A9E6A
    cpx.w #$0002                ; $8A9E6C
    beq CODE_8A9E7A             ; $8A9E6F
    cpx.w #$0003                ; $8A9E71
    db $F0,$08 ; beq            ; $8A9E74
    jsr.w $BE2E                 ; $8A9E76
    rtl                         ; $8A9E79
CODE_8A9E7A:
    jsr.w $C056                 ; $8A9E7A
    rtl                         ; $8A9E7D
    incbin !baserom:$51E7E..$51E82
CODE_8A9E82:
    lda.w $0008                 ; $8A9E82
    pha                         ; $8A9E85
    stz.w $0196                 ; $8A9E86
    stz.w $0198                 ; $8A9E89
    stz.w $019A                 ; $8A9E8C
    jsl.l $8A8001               ; $8A9E8F
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
    jsl.l $8081C8               ; $8A9EAE
    jsr.w $A06D                 ; $8A9EB2
    jsl.l $8A8DEC               ; $8A9EB5
    jsr.w $BA55                 ; $8A9EB9
    jsr.w $B994                 ; $8A9EBC
    jsr.w $BACB                 ; $8A9EBF
    jsr.w $B91A                 ; $8A9EC2
    jsl.l $8A8EEF               ; $8A9EC5
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
    jsr.w $A4BE                 ; $8A9F2C
    stz.b $6C                   ; $8A9F2F
    jsr.w $B0E1                 ; $8A9F31
    lda.w #$0A8C                ; $8A9F34
    sta.w $0D7E                 ; $8A9F37
CODE_8A9F3A:
    lda.w #$0001                ; $8A9F3A
    jsl.l $8ABA90               ; $8A9F3D
    lda.w #$CBA7                ; $8A9F41
    jsr.w $A2AF                 ; $8A9F44
    jsr.w $9F76                 ; $8A9F47
    lda.b $6C                   ; $8A9F4A
    beq CODE_8A9F3A             ; $8A9F4C
    jsr.w $9F64                 ; $8A9F4E
    stz.w $0198                 ; $8A9F51
    lda.w #$FFC0                ; $8A9F54
    sta.w $0196                 ; $8A9F57
CODE_8A9F5A:
    lda.w $019A                 ; $8A9F5A
    bne CODE_8A9F5A             ; $8A9F5D
    pla                         ; $8A9F5F
    sta.w $0008                 ; $8A9F60
    rtl                         ; $8A9F63
    lda.b $52                   ; $8A9F64
    beq CODE_8A9F6F             ; $8A9F66
    incbin !baserom:$51F68..$51F6F
CODE_8A9F6F:
    lda.w #$1000                ; $8A9F6F
    trb.w $03DC                 ; $8A9F72
    rts                         ; $8A9F75
    dec.w $0D7E                 ; $8A9F76
    bne CODE_8A9FA0             ; $8A9F79
    incbin !baserom:$51F7B..$51FA0
CODE_8A9FA0:
    rts                         ; $8A9FA0
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
    stz.b $56                   ; $8AA005
    stz.b $58                   ; $8AA007
    lda.w $0018                 ; $8AA009
    jsr.w $A022                 ; $8AA00C
    lda.w $0014                 ; $8AA00F
    jsr.w $A022                 ; $8AA012
    lda.w $0010                 ; $8AA015
    jsr.w $A022                 ; $8AA018
    lda.w $000C                 ; $8AA01B
    jsr.w $A022                 ; $8AA01E
    rts                         ; $8AA021
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
    lda.b $8E                   ; $8AA050
    beq CODE_8AA069             ; $8AA052
    jsr.w $BA7D                 ; $8AA054
    jsl.l $8A8E1B               ; $8AA057
    inc.w $0164                 ; $8AA05B
    lda.w $0164                 ; $8AA05E
    and.w #$0007                ; $8AA061
    bne CODE_8AA069             ; $8AA064
    inc.w $0166                 ; $8AA066
CODE_8AA069:
    jsr.w $A005                 ; $8AA069
    rtl                         ; $8AA06C
    jsr.w $A09F                 ; $8AA06D
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
    phx                         ; $8AA0F9
    ldy.b $7E                   ; $8AA0FA
    jsr.w $A27D                 ; $8AA0FC
    ldy.b $7C                   ; $8AA0FF
    jsr.w $A296                 ; $8AA101
    ldy.b $7A                   ; $8AA104
    jsr.w $A264                 ; $8AA106
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
    jsr.w $A25D                 ; $8AA128
    lda.w #$FFFF                ; $8AA12B
    sta.w $0500,y               ; $8AA12E
    lda.w #$3000                ; $8AA131
    sta.w $0800,y               ; $8AA134
    lda.w $0D60,x               ; $8AA137
    sta.w $0600,y               ; $8AA13A
    lda.w $0D68,x               ; $8AA13D
    sta.w $0700,y               ; $8AA140
    jsr.w $A25D                 ; $8AA143
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
    jsr.w $A25D                 ; $8AA161
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
    jsr.w $A216                 ; $8AA19B
    bra CODE_8AA18F             ; $8AA19E
CODE_8AA1A0:
    adc.w #$0008                ; $8AA1A0
    clc                         ; $8AA1A3
    adc.w $0D60,x               ; $8AA1A4
    sec                         ; $8AA1A7
    sbc.w #$0008                ; $8AA1A8
    sta.w $0D60,x               ; $8AA1AB
    jsr.w $A25D                 ; $8AA1AE
    lda.w #$FFFF                ; $8AA1B1
    sta.w $0500,y               ; $8AA1B4
    lda.w #$7000                ; $8AA1B7
    sta.w $0800,y               ; $8AA1BA
    lda.w $0D60,x               ; $8AA1BD
    sta.w $0600,y               ; $8AA1C0
    lda.w $0D68,x               ; $8AA1C3
    sta.w $0700,y               ; $8AA1C6
    jsr.w $A25D                 ; $8AA1C9
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
    jsr.w $A25D                 ; $8AA1E7
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
    jsr.w $A25D                 ; $8AA216
    lda.w #$FFFF                ; $8AA219
    sta.w $0500,y               ; $8AA21C
    lda.w $0D60,x               ; $8AA21F
    sta.w $0600,y               ; $8AA222
    lda.w $0D68,x               ; $8AA225
    sta.w $0700,y               ; $8AA228
    lda.w #$3002                ; $8AA22B
    sta.w $0800,y               ; $8AA22E
    jsr.w $A25D                 ; $8AA231
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
    ldy.b $8C                   ; $8AA25D
    inc.b $8C                   ; $8AA25F
    inc.b $8C                   ; $8AA261
    rts                         ; $8AA263
    lda.w $0D88,x               ; $8AA264
    sec                         ; $8AA267
    sbc.w $0D80,x               ; $8AA268
    beq CODE_8AA27C             ; $8AA26B
    incbin !baserom:$5226D..$5227C
CODE_8AA27C:
    rts                         ; $8AA27C
CODE_8AA27D:
    lda.w $0D78,x               ; $8AA27D
    sec                         ; $8AA280
    sbc.w $0D70,x               ; $8AA281
    beq CODE_8AA295             ; $8AA284
    bmi CODE_8AA28F             ; $8AA286
    inc.w $0D70,x               ; $8AA288
    dey                         ; $8AA28B
    bne CODE_8AA27D             ; $8AA28C
    rts                         ; $8AA28E
CODE_8AA28F:
    dec.w $0D70,x               ; $8AA28F
    dey                         ; $8AA292
    bne CODE_8AA27D             ; $8AA293
CODE_8AA295:
    rts                         ; $8AA295
CODE_8AA296:
    lda.w $0D90,x               ; $8AA296
    sec                         ; $8AA299
    sbc.w $0D68,x               ; $8AA29A
    beq CODE_8AA2AE             ; $8AA29D
    bmi CODE_8AA2A8             ; $8AA29F
    inc.w $0D68,x               ; $8AA2A1
    dey                         ; $8AA2A4
    bne CODE_8AA296             ; $8AA2A5
    rts                         ; $8AA2A7
CODE_8AA2A8:
    dec.w $0D68,x               ; $8AA2A8
    dey                         ; $8AA2AB
    bne CODE_8AA296             ; $8AA2AC
CODE_8AA2AE:
    rts                         ; $8AA2AE
    sta.b $4A                   ; $8AA2AF
    jsr.w $A47E                 ; $8AA2B1
    ldx.w #$0000                ; $8AA2B4
    jsr.w $A0F9                 ; $8AA2B7
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
    jsr.w $A32B                 ; $8AA2D6
    lda.w $1516,x               ; $8AA2D9
    and.w #$0002                ; $8AA2DC
    beq CODE_8AA2F7             ; $8AA2DF
    incbin !baserom:$522E1..$522F7
CODE_8AA2F7:
    stz.b $68                   ; $8AA2F7
    lda.b ($4A),y               ; $8AA2F9
    jsr.w $A4CF                 ; $8AA2FB
    jsr.w $B6AA                 ; $8AA2FE
    jsr.w $A37A                 ; $8AA301
    lda.w $1516,x               ; $8AA304
    and.w #$0001                ; $8AA307
    beq CODE_8AA31F             ; $8AA30A
    jsr.w $A469                 ; $8AA30C
    jsr.w $A381                 ; $8AA30F
    lda.w $1516,x               ; $8AA312
    and.w #$0080                ; $8AA315
    bne CODE_8AA31F             ; $8AA318
    jsr.w $A419                 ; $8AA31A
    bcs CODE_8AA32A             ; $8AA31D
CODE_8AA31F:
    clc                         ; $8AA31F
    lda.b $4A                   ; $8AA320
    adc.w #$0008                ; $8AA322
    sta.b $4A                   ; $8AA325
    jmp.w $A2C9                 ; $8AA327
CODE_8AA32A:
    rts                         ; $8AA32A
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
    incbin !baserom:$52345..$5234C
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
    inc.b $3C                   ; $8AA37A
    inc.b $3C                   ; $8AA37C
    inc.b $3C                   ; $8AA37E
    rts                         ; $8AA380
    lda.w #$0800                ; $8AA381
    trb.w $000E                 ; $8AA384
    db $D0,$39 ; bne            ; $8AA387
    lda.w #$0800                ; $8AA389
    trb.w $0012                 ; $8AA38C
    db $D0,$31 ; bne            ; $8AA38F
    lda.w #$0800                ; $8AA391
    trb.w $0016                 ; $8AA394
    db $D0,$29 ; bne            ; $8AA397
    lda.w #$0800                ; $8AA399
    trb.w $001A                 ; $8AA39C
    db $D0,$21 ; bne            ; $8AA39F
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
    incbin !baserom:$523C2..$523F4
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
    jsr.w $A48D                 ; $8AA414
CODE_8AA417:
    ply                         ; $8AA417
    rts                         ; $8AA418
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
    jsr.w $A49E                 ; $8AA456
    jsr.w $A466                 ; $8AA459
    ply                         ; $8AA45C
    plx                         ; $8AA45D
    pla                         ; $8AA45E
    sta.b $4A                   ; $8AA45F
    pla                         ; $8AA461
    sta.b $62                   ; $8AA462
    sec                         ; $8AA464
    rts                         ; $8AA465
    jmp.w ($015C)               ; $8AA466
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
    pha                         ; $8AA48D
    sep.b #$20                  ; $8AA48E
    lda.b #$11                  ; $8AA490
    sta.b $FB                   ; $8AA492
    rep.b #$20                  ; $8AA494
    lda.w #$0A8C                ; $8AA496
    sta.w $0D7E                 ; $8AA499
    pla                         ; $8AA49C
    rts                         ; $8AA49D
    pha                         ; $8AA49E
    sep.b #$20                  ; $8AA49F
    lda.b #$13                  ; $8AA4A1
    sta.b $FB                   ; $8AA4A3
    rep.b #$20                  ; $8AA4A5
    lda.w #$0A8C                ; $8AA4A7
    sta.w $0D7E                 ; $8AA4AA
    pla                         ; $8AA4AD
    rts                         ; $8AA4AE
    incbin !baserom:$524AF..$524BE
    sta.b $4A                   ; $8AA4BE
    ldy.w #$0000                ; $8AA4C0
CODE_8AA4C3:
    lda.b ($4A),y               ; $8AA4C3
    beq CODE_8AA4CE             ; $8AA4C5
    jsr.w $B680                 ; $8AA4C7
    iny                         ; $8AA4CA
    iny                         ; $8AA4CB
    bra CODE_8AA4C3             ; $8AA4CC
CODE_8AA4CE:
    rts                         ; $8AA4CE
    sta.b $44                   ; $8AA4CF
    phx                         ; $8AA4D1
    phy                         ; $8AA4D2
    lda.w $1516,x               ; $8AA4D3
    and.w #$003C                ; $8AA4D6
    lsr a                       ; $8AA4D9
    lsr a                       ; $8AA4DA
    tax                         ; $8AA4DB
    beq CODE_8AA4F6             ; $8AA4DC
    incbin !baserom:$524DE..$524F6
CODE_8AA4F6:
    lda.b $44                   ; $8AA4F6
    ply                         ; $8AA4F8
    plx                         ; $8AA4F9
    rts                         ; $8AA4FA
    lda.w #$CBD2                ; $8AA4FB
    jsr.w $B659                 ; $8AA4FE
    rts                         ; $8AA501
    incbin !baserom:$52502..$530E1
    lda.w $1528                 ; $8AB0E1
    and.w #$FFC3                ; $8AB0E4
    sta.w $1528                 ; $8AB0E7
    lda.l $8103DC               ; $8AB0EA
    and.w #$0400                ; $8AB0EE
    ldy.w #$0008                ; $8AB0F1
    jsr.w $B15E                 ; $8AB0F4
    ora.w $1528                 ; $8AB0F7
    sta.w $1528                 ; $8AB0FA
    lda.w $1526                 ; $8AB0FD
    and.w #$FFC3                ; $8AB100
    sta.w $1526                 ; $8AB103
    lda.l $8103DC               ; $8AB106
    and.w #$0300                ; $8AB10A
    ldy.w #$0006                ; $8AB10D
    jsr.w $B15E                 ; $8AB110
    ora.w $1526                 ; $8AB113
    sta.w $1526                 ; $8AB116
    rts                         ; $8AB119
    incbin !baserom:$5311A..$5315E
    cpy.w #$0000                ; $8AB15E
    beq CODE_8AB167             ; $8AB161
CODE_8AB163:
    lsr a                       ; $8AB163
    dey                         ; $8AB164
    bne CODE_8AB163             ; $8AB165
CODE_8AB167:
    rts                         ; $8AB167
    incbin !baserom:$53168..$53174
    sep.b #$20                  ; $8AB174
    lda.b #$10                  ; $8AB176
    sta.w $03DC                 ; $8AB178
    rep.b #$20                  ; $8AB17B
    inc.b $6C                   ; $8AB17D
    rts                         ; $8AB17F
    incbin !baserom:$53180..$5318C
    lda.w #$0001                ; $8AB18C
    jsl.l $8ABA90               ; $8AB18F
    stz.w $420C                 ; $8AB193
    rts                         ; $8AB196
    incbin !baserom:$53197..$53212
    pha                         ; $8AB212
    lda.w #$0001                ; $8AB213
    jsl.l $8ABA90               ; $8AB216
    pla                         ; $8AB21A
    rts                         ; $8AB21B
    incbin !baserom:$5321C..$53659
    sta.b $62                   ; $8AB659
    jsr.w $B91A                 ; $8AB65B
CODE_8AB65E:
    jsr.w $9F76                 ; $8AB65E
    lda.b $6C                   ; $8AB661
    bne CODE_8AB67F             ; $8AB663
    lda.w #$0001                ; $8AB665
    jsl.l $8ABA90               ; $8AB668
    jsr.w $9FA1                 ; $8AB66C
    lda.b $62                   ; $8AB66F
    jsr.w $A2AF                 ; $8AB671
    lda.b $5E                   ; $8AB674
    beq CODE_8AB65E             ; $8AB676
    incbin !baserom:$53678..$5367F
CODE_8AB67F:
    rts                         ; $8AB67F
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
    jsr.w $B83C                 ; $8AB6EF
    iny                         ; $8AB6F2
    bra CODE_8AB6DB             ; $8AB6F3
CODE_8AB6F5:
    rep.b #$20                  ; $8AB6F5
    lda.b $86                   ; $8AB6F7
    bne CODE_8AB6FE             ; $8AB6F9
    jsr.w $B700                 ; $8AB6FB
CODE_8AB6FE:
    plx                         ; $8AB6FE
    rts                         ; $8AB6FF
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
    incbin !baserom:$53721..$5383C
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
    jsr.w $B909                 ; $8AB853
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
    jsr.w $B909                 ; $8AB8D8
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
    jsr.w $B942                 ; $8AB930
    lda.b $78                   ; $8AB933
    beq CODE_8AB93E             ; $8AB935
    lda.w #$0001                ; $8AB937
    jsl.l $8ABA90               ; $8AB93A
CODE_8AB93E:
    dex                         ; $8AB93E
    bne CODE_8AB92E             ; $8AB93F
    rts                         ; $8AB941
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
    ldy.w #$0800                ; $8AB994
    ldx.w #$FEC3                ; $8AB997
    lda.w #$0088                ; $8AB99A
    jsl.l $80837D               ; $8AB99D
    ldy.w #$1000                ; $8AB9A1
    ldx.w #$F87F                ; $8AB9A4
    lda.w #$0088                ; $8AB9A7
    jsl.l $80837D               ; $8AB9AA
    rts                         ; $8AB9AE
    incbin !baserom:$539AF..$53A55
    ldx.w #$EE52                ; $8ABA55
    ldy.w #$4000                ; $8ABA58
    lda.w #$0096                ; $8ABA5B
    jsl.l $80837D               ; $8ABA5E
    ldx.w #$FA8B                ; $8ABA62
    ldy.w #$7000                ; $8ABA65
    lda.w #$0085                ; $8ABA68
    jsl.l $80837D               ; $8ABA6B
    lda.w #$8A00                ; $8ABA6F
    ldx.w #$C658                ; $8ABA72
    ldy.w #$0200                ; $8ABA75
    jsl.l $8083CB               ; $8ABA78
    rts                         ; $8ABA7C
    lda.w #$007F                ; $8ABA7D
    sta.b $14                   ; $8ABA80
    ldx.w #$0000                ; $8ABA82
    ldy.w #$3000                ; $8ABA85
    lda.w #$0800                ; $8ABA88
    jsl.l $8083AD               ; $8ABA8B
    rts                         ; $8ABA8F
    clc                         ; $8ABA90
    adc.l $810024               ; $8ABA91
CODE_8ABA95:
    cmp.l $810024               ; $8ABA95
    bne CODE_8ABA95             ; $8ABA99
    rtl                         ; $8ABA9B
    incbin !baserom:$53A9C..$53ACB
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
    stz.w $0198                 ; $8ABAE3
    lda.w #$FF80                ; $8ABAE6
    sta.w $0196                 ; $8ABAE9
CODE_8ABAEC:
    lda.w $0196                 ; $8ABAEC
    bne CODE_8ABAEC             ; $8ABAEF
    rts                         ; $8ABAF1
    lda.w #$0F00                ; $8ABAF2
    sta.w $0198                 ; $8ABAF5
    lda.w #$0080                ; $8ABAF8
    sta.w $0196                 ; $8ABAFB
    rts                         ; $8ABAFE
    incbin !baserom:$53AFF..$53B5C
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
    jsr.w $BC7F                 ; $8ABB99
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
    jsr.w $B15E                 ; $8ABBC2
    ply                         ; $8ABBC5
    eor.w $BC3D,y               ; $8ABBC6
    jsr.w $BBF4                 ; $8ABBC9
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
    jsr.w $BC32                 ; $8ABC29
    dey                         ; $8ABC2C
    bne CODE_8ABBF6             ; $8ABC2D
    pla                         ; $8ABC2F
    ply                         ; $8ABC30
    rts                         ; $8ABC31
    pha                         ; $8ABC32
    lda.b $92                   ; $8ABC33
    clc                         ; $8ABC35
    adc.w $0DCA                 ; $8ABC36
    sta.b $92                   ; $8ABC39
    pla                         ; $8ABC3B
    rts                         ; $8ABC3C
    incbin !baserom:$53C3D..$53C7F
    ldx.w #$001E                ; $8ABC7F
CODE_8ABC82:
    stz.w $0DAC,x               ; $8ABC82
    dex                         ; $8ABC85
    dex                         ; $8ABC86
    bpl CODE_8ABC82             ; $8ABC87
    rts                         ; $8ABC89
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
    jsr.w $BB5C                 ; $8ABC9E
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
    incbin !baserom:$53CEA..$53E2E
    lda.w $0008                 ; $8ABE2E
    pha                         ; $8ABE31
    stz.w $0196                 ; $8ABE32
    stz.w $0198                 ; $8ABE35
    stz.w $019A                 ; $8ABE38
    lda.w #$8934                ; $8ABE3B
    sta.w $0008                 ; $8ABE3E
    jsl.l $8A8001               ; $8ABE41
    stz.b $8E                   ; $8ABE45
    ldx.w #$C4B0                ; $8ABE47
    lda.w #$008A                ; $8ABE4A
    jsl.l $8081C8               ; $8ABE4D
    jsl.l $8A8DEC               ; $8ABE51
    jsr.w $BAE3                 ; $8ABE55
    jsr.w $B18C                 ; $8ABE58
    jsr.w $B91A                 ; $8ABE5B
    stz.b $8E                   ; $8ABE5E
    ldx.w #$8601                ; $8ABE60
    ldy.w #$1000                ; $8ABE63
    lda.w #$0088                ; $8ABE66
    jsl.l $80837D               ; $8ABE69
    jsr.w $BA55                 ; $8ABE6D
    lda.w #$8560                ; $8ABE70
    ldx.w #$FA37                ; $8ABE73
    ldy.w #$0040                ; $8ABE76
    jsl.l $8083CB               ; $8ABE79
    jsr.w $B91A                 ; $8ABE7D
    jsr.w $BAF2                 ; $8ABE80
    jsr.w $B212                 ; $8ABE83
    inc.b $8E                   ; $8ABE86
    inc.w $0D7C                 ; $8ABE88
    jsl.l $8A8EEF               ; $8ABE8B
    jsr.w $B981                 ; $8ABE8F
    jsr.w $BF4E                 ; $8ABE92
    lda.w #$FC5F                ; $8ABE95
    sta.w $0008                 ; $8ABE98
    inc.b $8E                   ; $8ABE9B
    jsr.w $BB99                 ; $8ABE9D
    jsr.w $BC8A                 ; $8ABEA0
    jsr.w $BF76                 ; $8ABEA3
    ldx.w #$0000                ; $8ABEA6
    ldy.w #$001F                ; $8ABEA9
    stx.b $3E                   ; $8ABEAC
    sty.b $40                   ; $8ABEAE
    lda.w #$0010                ; $8ABEB0
    sta.b $3C                   ; $8ABEB3
    stz.b $68                   ; $8ABEB5
    lda.w #$0001                ; $8ABEB7
    sta.b $86                   ; $8ABEBA
    jsr.w $BF2A                 ; $8ABEBC
    lda.w #$0D60                ; $8ABEBF
    jsr.w $B6AA                 ; $8ABEC2
    lda.w #$0008                ; $8ABEC5
    jsl.l $8ABA90               ; $8ABEC8
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
    jsr.w $C024                 ; $8ABF4E
    lda.w $038E                 ; $8ABF51
    sta.b $44                   ; $8ABF54
    lda.w $0390                 ; $8ABF56
    sta.b $46                   ; $8ABF59
    jsr.w $BF9E                 ; $8ABF5B
    lda.b $42                   ; $8ABF5E
    sta.w $038E                 ; $8ABF60
    lda.w $03BD                 ; $8ABF63
    sta.b $44                   ; $8ABF66
    lda.w $03BF                 ; $8ABF68
    sta.b $46                   ; $8ABF6B
    jsr.w $BF9E                 ; $8ABF6D
    lda.b $42                   ; $8ABF70
    sta.w $03BD                 ; $8ABF72
    rts                         ; $8ABF75
    lda.w $038E                 ; $8ABF76
    sta.b $42                   ; $8ABF79
    jsr.w $BFD0                 ; $8ABF7B
    lda.b $44                   ; $8ABF7E
    sta.w $038E                 ; $8ABF80
    lda.b $46                   ; $8ABF83
    sta.w $0390                 ; $8ABF85
    lda.w $03BD                 ; $8ABF88
    sta.b $42                   ; $8ABF8B
    jsr.w $BFD0                 ; $8ABF8D
    lda.b $44                   ; $8ABF90
    sta.w $03BD                 ; $8ABF92
    lda.b $46                   ; $8ABF95
    sta.w $03BF                 ; $8ABF97
    jsr.w $C03D                 ; $8ABF9A
    rts                         ; $8ABF9D
    stz.b $42                   ; $8ABF9E
    ldx.w #$0000                ; $8ABFA0
CODE_8ABFA3:
    jsr.w $BFBC                 ; $8ABFA3
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
    phy                         ; $8ABFBC
    ldy.w #$0004                ; $8ABFBD
CODE_8ABFC0:
    lsr.b $46                   ; $8ABFC0
    ror.b $44                   ; $8ABFC2
    ror a                       ; $8ABFC4
    dey                         ; $8ABFC5
    bne CODE_8ABFC0             ; $8ABFC6
    ldy.w #$000C                ; $8ABFC8
    jsr.w $B15E                 ; $8ABFCB
    ply                         ; $8ABFCE
    rts                         ; $8ABFCF
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
    incbin !baserom:$54004..$54024
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
    lda.w $0008                 ; $8AC056
    pha                         ; $8AC059
    stz.w $0196                 ; $8AC05A
    stz.w $0198                 ; $8AC05D
    stz.w $019A                 ; $8AC060
    jsl.l $8A8001               ; $8AC063
    stz.b $8E                   ; $8AC067
    ldx.w #$C4B0                ; $8AC069
    lda.w #$008A                ; $8AC06C
    jsl.l $8081C8               ; $8AC06F
    jsl.l $8A8DEC               ; $8AC073
    jsr.w $BA55                 ; $8AC077
    ldy.w #$0800                ; $8AC07A
    ldx.w #$F861                ; $8AC07D
    lda.w #$0087                ; $8AC080
    jsl.l $80837D               ; $8AC083
    jsr.w $C24B                 ; $8AC087
    ldy.w #$0000                ; $8AC08A
    ldx.w #$F5BC                ; $8AC08D
    lda.w #$0087                ; $8AC090
    jsl.l $80837D               ; $8AC093
    jsl.l $8A8EEF               ; $8AC097
    jsr.w $B981                 ; $8AC09B
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
    jsr.w $C19C                 ; $8AC0BE
    jsr.w $C0F2                 ; $8AC0C1
    lda.w #$0F00                ; $8AC0C4
    sta.w $0198                 ; $8AC0C7
    lda.w #$0080                ; $8AC0CA
    sta.w $0196                 ; $8AC0CD
    ldx.w #$00B4                ; $8AC0D0
CODE_8AC0D3:
    phx                         ; $8AC0D3
    lda.w #$0001                ; $8AC0D4
    jsl.l $8ABA90               ; $8AC0D7
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
    stz.b $90                   ; $8AC0F2
    inc.b $90                   ; $8AC0F4
    ldx.w #$000C                ; $8AC0F6
    ldy.w #$0006                ; $8AC0F9
    lda.w #$C44F                ; $8AC0FC
    jsr.w $C137                 ; $8AC0FF
    dec.b $90                   ; $8AC102
    ldx.w #$0006                ; $8AC104
    ldy.w #$0016                ; $8AC107
    lda.w #$C458                ; $8AC10A
    jsr.w $C137                 ; $8AC10D
    ldx.w #$0005                ; $8AC110
    ldy.w #$0017                ; $8AC113
    lda.w #$C46D                ; $8AC116
    jsr.w $C137                 ; $8AC119
    ldx.w #$0005                ; $8AC11C
    ldy.w #$0018                ; $8AC11F
    lda.w #$C484                ; $8AC122
    jsr.w $C137                 ; $8AC125
    inc.b $90                   ; $8AC128
    ldx.w #$0006                ; $8AC12A
    ldy.w #$001A                ; $8AC12D
    lda.w #$C49B                ; $8AC130
    jsr.w $C137                 ; $8AC133
    rts                         ; $8AC136
    stx.b $3A                   ; $8AC137
    sty.b $3C                   ; $8AC139
    jsr.w $C13F                 ; $8AC13B
    rts                         ; $8AC13E
    sta.b $48                   ; $8AC13F
    ldy.w #$0000                ; $8AC141
CODE_8AC144:
    lda.b ($48),y               ; $8AC144
    and.w #$00FF                ; $8AC146
    beq CODE_8AC156             ; $8AC149
    cmp.w #$0080                ; $8AC14B
    db $F0,$07 ; beq            ; $8AC14E
    jsr.w $C278                 ; $8AC150
    iny                         ; $8AC153
    bra CODE_8AC144             ; $8AC154
CODE_8AC156:
    rts                         ; $8AC156
    incbin !baserom:$54157..$5415D
    ldx.w #$0000                ; $8AC15D
CODE_8AC160:
    lda.l $7F7D00,x             ; $8AC160
    and.w #$00FF                ; $8AC164
    beq CODE_8AC174             ; $8AC167
    cmp.w #$0080                ; $8AC169
    beq CODE_8AC183             ; $8AC16C
    jsr.w $C278                 ; $8AC16E
CODE_8AC171:
    inx                         ; $8AC171
    bra CODE_8AC160             ; $8AC172
CODE_8AC174:
    ldx.w #$C224                ; $8AC174
    bit.w $1B37                 ; $8AC177
    bpl CODE_8AC17F             ; $8AC17A
    incbin !baserom:$5417C..$5417F
CODE_8AC17F:
    jsr.w $C1C6                 ; $8AC17F
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
    jsr.w $B6AA                 ; $8AC20A
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
    jsl.l $8083CB               ; $8AC266
    lda.w #$8A60                ; $8AC26A
    ldx.w #$C984                ; $8AC26D
    ldy.w #$0040                ; $8AC270
    jsl.l $8083CB               ; $8AC273
    rts                         ; $8AC277
    php                         ; $8AC278
    phy                         ; $8AC279
    phx                         ; $8AC27A
    rep.b #$20                  ; $8AC27B
    and.w #$00FF                ; $8AC27D
    ldy.b $3A                   ; $8AC280
    sty.b $36                   ; $8AC282
    ldy.b $3C                   ; $8AC284
    sty.b $38                   ; $8AC286
    jsr.w $C2CC                 ; $8AC288
    jsr.w $C2A2                 ; $8AC28B
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
    lda.w $03DC                 ; $8AC2DD
    and.w #$0300                ; $8AC2E0
    db $F0,$0A ; beq            ; $8AC2E3
    cmp.w #$0100                ; $8AC2E5
    beq CODE_8AC311             ; $8AC2E8
    incbin !baserom:$542EA..$54311
CODE_8AC311:
    ldx.w #$0000                ; $8AC311
    lda.l $8ACADE,x             ; $8AC314
    and.w #$00FF                ; $8AC318
    cmp.w $0444                 ; $8AC31B
    beq CODE_8AC328             ; $8AC31E
    incbin !baserom:$54320..$54328
CODE_8AC328:
    lda.l $8ACADE,x             ; $8AC328
    and.w #$00FF                ; $8AC32C
    sta.w $0444                 ; $8AC32F
    rtl                         ; $8AC332
    incbin !baserom:$54333..$58000
