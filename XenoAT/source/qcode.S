.text

.global qcode
.global qcode_end
.global uploader
.global uploader_end
.global launcher
.global launcher_end


.align 2
#--------------------------------------
 qcode:
#--------------------------------------
.incbin "XenoAT/source/qcode.bin"
qcode_end:

.align 2
#--------------------------------------
 uploader:
#--------------------------------------
.incbin "XenoAT/source/uploader.bin"
uploader_end:

.align 2
#--------------------------------------
 launcher:
#--------------------------------------
	.incbin "XenoAT/source/XenoLauncher.bin"
launcher_end:


