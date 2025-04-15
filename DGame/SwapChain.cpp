#include "pch.h"
#include "SwapChain.h"

DDing::SwapChain::SwapChain(DDing::Context& context) : context(&context), 
swapChain(nullptr)
{
	create();
}

void DDing::SwapChain::create()
{
}
