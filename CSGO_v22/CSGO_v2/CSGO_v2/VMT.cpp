#include "VMT.h"
#include <iostream>

void VMT_t::init(uintptr_t* base)
{
    this->base = base;
    this->length = this->calcLength();
    this->initVMT();
}

void VMT_t::init(VMT_t& vmt)
{
    this->base = vmt.getBase();
    this->length = vmt.getLength();
    this->initVMT(vmt);
}

void VMT_t::initVMT(VMT_t& vmt)
{
    this->VMT = std::make_unique<uintptr_t[]>(vmt.getLength());
    std::copy(vmt.getBase(), vmt.getBase() + vmt.getLength(), this->VMT.get());
}

void VMT_t::initVMT()
{
    this->VMT = std::make_unique<uintptr_t[]>(this->length);
    std::copy(this->base, this->base + this->length, this->VMT.get());
}

void* VMT_t::getVirtualFunction(unsigned int index)
{
    return (*static_cast<void***>((void*)this->base))[index];
}

bool VMT_t::setIndex(unsigned int index, void* newFunction)
{
    uintptr_t* dest = this->getPtr() + index;
    DWORD oldProtect;
    if (VirtualProtect(dest, sizeof(dest), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *dest = uintptr_t(newFunction);
        VirtualProtect(dest, sizeof(dest), oldProtect, nullptr);
        return true;
    }
    return false;
}

bool VMT_t::swapVMT(VMT_t& newVMT)
{
    uintptr_t* dest = this->getPtr();
    DWORD oldProtect;
    if (VirtualProtect(dest, sizeof(dest), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::copy(newVMT.getBase(), newVMT.getBase() + newVMT.getLength(), dest);
        VirtualProtect(dest, sizeof(dest), oldProtect, nullptr);
        return true;
    }
    return false;
}

unsigned int VMT_t::calcLength()
{
    unsigned int size = 0;
    MEMORY_BASIC_INFORMATION MBI;
    do {
        size++;
    } while (VirtualQuery(LPCVOID(this->getPtr()[size]), &MBI, sizeof(MBI)) && MBI.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)); // Validate the pointer, if valid, proceed and increase size

    return size;
}
