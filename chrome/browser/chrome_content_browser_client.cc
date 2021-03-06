// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chrome_content_browser_client.h"

#include "base/command_line.h"
#include "chrome/app/breakpad_mac.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/character_encoding.h"
#include "chrome/browser/chrome_worker_message_filter.h"
#include "chrome/browser/debugger/devtools_handler.h"
#include "chrome/browser/desktop_notification_handler.h"
#include "chrome/browser/extensions/extension_message_handler.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/google/google_util.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/printing/printing_message_filter.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/renderer_host/chrome_render_message_filter.h"
#include "chrome/browser/renderer_host/chrome_render_view_host_observer.h"
#include "chrome/browser/renderer_host/text_input_client_message_filter.h"
#include "chrome/browser/search_engines/search_provider_install_state_message_filter.h"
#include "chrome/browser/spellcheck_message_filter.h"
#include "chrome/browser/ui/webui/chrome_web_ui_factory.h"
#include "chrome/common/child_process_logging.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "content/browser/renderer_host/browser_render_process_host.h"
#include "content/browser/renderer_host/render_view_host.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/worker_host/worker_process_host.h"

#if defined(OS_LINUX)
#include "base/linux_util.h"
#include "chrome/browser/crash_handler_host_linux.h"
#endif  // OS_LINUX

namespace chrome {

void ChromeContentBrowserClient::RenderViewHostCreated(
    RenderViewHost* render_view_host) {
  new ChromeRenderViewHostObserver(render_view_host);
  new DesktopNotificationHandler(render_view_host);
  new DevToolsHandler(render_view_host);
  new ExtensionMessageHandler(render_view_host);
}

void ChromeContentBrowserClient::PreCreateRenderView(
    RenderViewHost* render_view_host,
    Profile* profile,
    const GURL& url) {
  // Tell the RenderViewHost whether it will be used for an extension process.
  ExtensionService* service = profile->GetExtensionService();
  if (service) {
    bool is_extension_process = service->ExtensionBindingsAllowed(url);
    render_view_host->set_is_extension_process(is_extension_process);

    const Extension* installed_app = service->GetInstalledApp(url);
    if (installed_app) {
      service->SetInstalledAppForRenderer(
          render_view_host->process()->id(), installed_app);
    }
  }
}

void ChromeContentBrowserClient::BrowserRenderProcessHostCreated(
    BrowserRenderProcessHost* host) {
  int id = host->id();
  Profile* profile = host->profile();
  host->channel()->AddFilter(new ChromeRenderMessageFilter(
      id, profile, profile->GetRequestContextForRenderProcess(id)));
  host->channel()->AddFilter(new PrintingMessageFilter());
  host->channel()->AddFilter(
      new SearchProviderInstallStateMessageFilter(id, profile));
  host->channel()->AddFilter(new SpellCheckMessageFilter(id));
#if defined(OS_MACOSX)
  host->channel()->AddFilter(new TextInputClientMessageFilter(host->id()));
#endif
}

void ChromeContentBrowserClient::WorkerProcessHostCreated(
    WorkerProcessHost* host) {
  host->AddFilter(new ChromeWorkerMessageFilter(host));
}

content::WebUIFactory* ChromeContentBrowserClient::GetWebUIFactory() {
  return ChromeWebUIFactory::GetInstance();
}

GURL ChromeContentBrowserClient::GetEffectiveURL(Profile* profile,
                                                 const GURL& url) {
  // Get the effective URL for the given actual URL. If the URL is part of an
  // installed app, the effective URL is an extension URL with the ID of that
  // extension as the host. This has the effect of grouping apps together in
  // a common SiteInstance.
  if (!profile || !profile->GetExtensionService())
    return url;

  const Extension* extension =
      profile->GetExtensionService()->GetExtensionByWebExtent(url);
  if (!extension)
    return url;

  // If the URL is part of an extension's web extent, convert it to an
  // extension URL.
  return extension->GetResourceURL(url.path());
}

GURL ChromeContentBrowserClient::GetAlternateErrorPageURL(
    const TabContents* tab) {
  GURL url;
  // Disable alternate error pages when in OffTheRecord/Incognito mode.
  if (tab->profile()->IsOffTheRecord())
    return url;

  PrefService* prefs = tab->profile()->GetPrefs();
  DCHECK(prefs);
  if (prefs->GetBoolean(prefs::kAlternateErrorPagesEnabled)) {
    url = google_util::AppendGoogleLocaleParam(
        GURL(google_util::kLinkDoctorBaseURL));
    url = google_util::AppendGoogleTLDParam(url);
  }
  return url;
}

std::string ChromeContentBrowserClient::GetCanonicalEncodingNameByAliasName(
    const std::string& alias_name) {
  return CharacterEncoding::GetCanonicalEncodingNameByAliasName(alias_name);
}

void ChromeContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line, int child_process_id) {
#if defined(USE_LINUX_BREAKPAD)
  if (IsCrashReporterEnabled()) {
    command_line->AppendSwitchASCII(switches::kEnableCrashReporter,
        child_process_logging::GetClientId() + "," + base::GetLinuxDistro());
  }
#elif defined(OS_MACOSX)
  if (IsCrashReporterEnabled()) {
    command_line->AppendSwitchASCII(switches::kEnableCrashReporter,
                                    child_process_logging::GetClientId());
  }
#endif  // OS_MACOSX

  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type == switches::kExtensionProcess ||
      process_type == switches::kRendererProcess) {
    const CommandLine& browser_command_line = *CommandLine::ForCurrentProcess();
    FilePath user_data_dir =
        browser_command_line.GetSwitchValuePath(switches::kUserDataDir);
    if (!user_data_dir.empty())
      command_line->AppendSwitchPath(switches::kUserDataDir, user_data_dir);
#if defined(OS_CHROMEOS)
    const std::string& login_profile =
        browser_command_line.GetSwitchValueASCII(switches::kLoginProfile);
    if (!login_profile.empty())
      command_line->AppendSwitchASCII(switches::kLoginProfile, login_profile);
#endif

    RenderProcessHost* process = RenderProcessHost::FromID(child_process_id);

    PrefService* prefs = process->profile()->GetPrefs();
    // Currently this pref is only registered if applied via a policy.
    if (prefs->HasPrefPath(prefs::kDisable3DAPIs) &&
        prefs->GetBoolean(prefs::kDisable3DAPIs)) {
      // Turn this policy into a command line switch.
      command_line->AppendSwitch(switches::kDisable3DAPIs);
    }

    // Disable client-side phishing detection in the renderer if it is disabled
    // in the browser process.
    if (!g_browser_process->safe_browsing_detection_service())
      command_line->AppendSwitch(switches::kDisableClientSidePhishingDetection);
  }
}

std::string ChromeContentBrowserClient::GetApplicationLocale() {
  return g_browser_process->GetApplicationLocale();
}

#if defined(OS_LINUX)
int ChromeContentBrowserClient::GetCrashSignalFD(
    const std::string& process_type) {
  if (process_type == switches::kRendererProcess)
    return RendererCrashHandlerHostLinux::GetInstance()->GetDeathSignalSocket();

  if (process_type == switches::kPluginProcess)
    return PluginCrashHandlerHostLinux::GetInstance()->GetDeathSignalSocket();

  if (process_type == switches::kPpapiPluginProcess)
    return PpapiCrashHandlerHostLinux::GetInstance()->GetDeathSignalSocket();

  if (process_type == switches::kGpuProcess)
    return GpuCrashHandlerHostLinux::GetInstance()->GetDeathSignalSocket();

  return -1;
}
#endif

}  // namespace chrome
