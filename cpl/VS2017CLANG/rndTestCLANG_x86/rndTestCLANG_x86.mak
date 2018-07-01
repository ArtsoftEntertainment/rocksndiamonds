
all:    $(RTPATH)\rndTest.exe
		
$(RTPATH)\rndTest.exe : $(OBJPATH)\cpl\VS2017CLANG\rndTestCLANG_x86\$(PROJCONF)\rocksndiamonds.exe
     attrib -R $(RTPATH)\rndTest.exe
     copy /Y $(OBJPATH)\cpl\VS2017CLANG\rndTestCLANG_x86\$(PROJCONF)\rocksndiamonds.exe $(RTPATH)\rndTest.exe
     attrib +R $(RTPATH)\rndTest.exe

