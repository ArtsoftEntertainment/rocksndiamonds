# =============================================================================

RTPATH=P:
OBJPATH=P:
#PRJCONF=$(target)
PRJCONF=Release


all:	$(RTPATH)\rndTestCodeBlocksMinGw.exe
Win32:	$(RTPATH)\rndTestCodeBlocksMinGw.exe
Release:	$(RTPATH)\rndTestCodeBlocksMinGw.exe

$(RTPATH)\rndTestCodeBlocksMinGw.exe : $(OBJPATH)\cpl\CodeBlocks\rndTestVC_x86\$(PRJCONF)\rocksndiamonds.exe
	cp $(OBJPATH)\cpl\CodeBlocks\rndTestVC_x86\$(PRJCONF)\rocksndiamonds.exe $(RTPATH)\rndTestCodeBlocksMinGw.exe



