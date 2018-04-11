/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include <Windows.h>
#include <stdio.h>

#pragma warning (disable:4371)
#pragma warning (disable:4946)

using namespace Platform;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

ref class AppActivation : IFrameworkView
{
public:
	virtual void Initialize( _In_ CoreApplicationView^ applicationView )
	{	
		applicationView->Activated += ref new TypedEventHandler< CoreApplicationView^, IActivatedEventArgs^ >( this, &AppActivation::OnActivated );
	}
	virtual void SetWindow( _In_ CoreWindow^ window )	{}
	virtual void Load( _In_ String^ entryPoint )		{}
	virtual void Run()									{}
	virtual void Uninitialize()							{}

private:
	void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}
};

ref class SnippetViewSource : IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
	{
		return ref new AppActivation();
	}
};

void initPlatform()
{
	auto mySource = ref new SnippetViewSource();
	CoreApplication::Run(mySource);
}

extern int snippetMain(int, const char*const*);

void OutputDebugPrint(const char* format, ...)
{
	char buf[1024];

	va_list arg;
	va_start( arg, format );
	vsprintf_s(buf, sizeof buf, format, arg);
	va_end(arg);

	OutputDebugStringA(buf);
}

int main(Platform::Array<Platform::String^>^)
{
	initPlatform();
	snippetMain(0,NULL);
}
