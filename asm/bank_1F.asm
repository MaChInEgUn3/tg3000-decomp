; bank $9F (file offset $0F8000)
org $9F8000

    incbin !baserom:$F8000..$100000
