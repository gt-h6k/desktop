// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.220110.5

#pragma once
#ifndef WINRT_CfApiShellExtensions_0_H
#define WINRT_CfApiShellExtensions_0_H
WINRT_EXPORT namespace winrt::Windows::Storage::Provider
{
    struct IStorageProviderItemPropertySource;
}
WINRT_EXPORT namespace winrt::CfApiShellExtensions
{
    struct CustomStateProvider;
}
namespace winrt::impl
{
    template <> struct category<winrt::CfApiShellExtensions::CustomStateProvider>{ using type = class_category; };
    template <> inline constexpr auto& name_v<winrt::CfApiShellExtensions::CustomStateProvider> = L"CfApiShellExtensions.CustomStateProvider";
    template <> struct default_interface<winrt::CfApiShellExtensions::CustomStateProvider>{ using type = winrt::Windows::Storage::Provider::IStorageProviderItemPropertySource; };
}
#endif
