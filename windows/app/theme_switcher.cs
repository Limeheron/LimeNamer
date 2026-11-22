using System;
using System.ComponentModel;
using System.Windows;
using Microsoft.Win32;

namespace limenamer;

public sealed class THEME_SWITCHER : IDisposable
{
    private const string PERSONALIZE_KEY =
        @"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize";

    private readonly Application app_;
    private readonly SETTINGS settings_;
    private ResourceDictionary? colors_;
    private bool dark_;

    public THEME_SWITCHER(Application app, SETTINGS settings)
    {
        app_ = app;
        settings_ = settings;
        settings_.PropertyChanged += on_settings_changed;
        SystemEvents.UserPreferenceChanged += on_preference_changed;
    }

    public static bool system_is_dark()
    {
        using RegistryKey? key = Registry.CurrentUser.OpenSubKey(PERSONALIZE_KEY);
        return key?.GetValue("AppsUseLightTheme") is int light && light == 0;
    }

    public void apply()
    {
        THEME_MODE mode = settings_.theme_mode;
        app_.ThemeMode = mode switch {
            THEME_MODE.light => ThemeMode.Light,
            THEME_MODE.dark => ThemeMode.Dark,
            _ => ThemeMode.System,
        };
        bool dark = mode == THEME_MODE.dark || (mode == THEME_MODE.system && system_is_dark());
        dark_ = dark;
        ResourceDictionary colors = new() {
            Source = new Uri($"pack://application:,,,/themes/{(dark ? "dark" : "light")}.xaml"),
        };
        if (colors_ != null) {
            app_.Resources.MergedDictionaries.Remove(colors_);
        }
        app_.Resources.MergedDictionaries.Add(colors);
        colors_ = colors;
    }

    public void Dispose()
    {
        settings_.PropertyChanged -= on_settings_changed;
        SystemEvents.UserPreferenceChanged -= on_preference_changed;
    }

    private void on_settings_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(SETTINGS.theme_mode)) {
            apply();
        }
    }

    private void on_preference_changed(object sender, UserPreferenceChangedEventArgs args)
    {
        if (settings_.theme_mode == THEME_MODE.system && system_is_dark() != dark_) {
            app_.Dispatcher.BeginInvoke(apply);
        }
    }
}
