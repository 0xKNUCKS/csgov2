#pragma once
#include <Windows.h>
#include <memory>

class VMT_t
{
public:
	VMT_t() {}
	VMT_t(uintptr_t* base) { init(base); }
	VMT_t(VMT_t& vmt) { init(vmt); }


	void init(uintptr_t* base);
	void init(VMT_t& vmt);
	void initVMT(VMT_t& vmt);

	unsigned int getLength() { return length; }
	uintptr_t* getBase() { return base; }
	uintptr_t* getPtr() { return *(uintptr_t**)base; } // getVMTptr
	std::unique_ptr<uintptr_t[]>&  getTable() { return VMT; } // getVMTtable
	void* getVirtualFunction(unsigned int index);

	bool setIndex(unsigned int index, void* newFunction);
	bool swapVMT(VMT_t& newVMT);
private:
	std::unique_ptr<uintptr_t[]> VMT = nullptr;
	uintptr_t* base = nullptr;
	unsigned int length = 0;
	unsigned int calcLength();
	void initVMT(); // Self Init
};

