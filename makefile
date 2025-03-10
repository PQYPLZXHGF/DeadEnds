all:
	cd DeadEndsLib; make
	cd MenuLibrary; make
	cd UseMenus; make
	cd RunScript; make
	cd PatchSex; make
	cd TestProgram; make
	cd RandomizeKeys; make
	cd Partition; make
	cd MultiBases; make
	cd GenerateSExpressions; make
	cd TestDates; make

clean:
	cd DeadEndsLib; make clean
	cd MenuLibrary; make clean
	cd UseMenus; make clean
	cd RunScript; make clean
	cd PatchSex; make clean
	cd TestProgram; make clean
	cd RandomizeKeys; make clean
	cd Partition; make clean
	cd MultiBases; make clean
	cd GenerateSExpressions; make clean
	cd TestDates; make clean
