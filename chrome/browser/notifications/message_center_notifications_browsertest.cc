// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/command_line.h"
#include "base/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/notifications/message_center_notification_manager.h"
#include "chrome/browser/notifications/notification.h"
#include "chrome/browser/notifications/notification_ui_manager.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/message_center_switches.h"
#include "ui/message_center/message_center_util.h"

class MessageCenterNotificationsTest : public InProcessBrowserTest {
 public:
  MessageCenterNotificationsTest() {}

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    // This switch enables the new piping of Notifications through Message
    // Center.
    command_line->AppendSwitch(
      message_center::switches::kEnableRichNotifications);
  }

  MessageCenterNotificationManager* manager() {
    return static_cast<MessageCenterNotificationManager*>(
        g_browser_process->notification_ui_manager());
  }

  message_center::MessageCenter* message_center() {
    return g_browser_process->message_center();
  }

  Profile* profile() { return browser()->profile(); }

  class TestDelegate : public NotificationDelegate {
   public:
    explicit TestDelegate(const std::string& id) : id_(id) {}

    virtual void Display() OVERRIDE { log_ += "Display_"; }
    virtual void Error() OVERRIDE { log_ += "Error_"; }
    virtual void Close(bool by_user) OVERRIDE {
      log_ += "Close_";
      log_ += ( by_user ? "by_user_" : "programmatically_");
    }
    virtual void Click() OVERRIDE { log_ += "Click_"; }
    virtual void ButtonClick(int button_index) OVERRIDE {
      log_ += "ButtonClick_";
      log_ += base::IntToString(button_index) + "_";
    }
    virtual std::string id() const OVERRIDE { return id_; }
    virtual content::RenderViewHost* GetRenderViewHost() const OVERRIDE {
      return NULL;
    }

    const std::string& log() { return log_; }

   private:
    virtual ~TestDelegate() {}
    std::string id_;
    std::string log_;

    DISALLOW_COPY_AND_ASSIGN(TestDelegate);
  };

  Notification CreateTestNotification(const std::string& id,
                                      TestDelegate** delegate = NULL) {
    TestDelegate* new_delegate = new TestDelegate(id);
    if (delegate) {
      *delegate = new_delegate;
      new_delegate->AddRef();
    }

    return Notification(GURL("chrome-test://testing/"),
                        GURL(),
                        ASCIIToUTF16("title"),
                        ASCIIToUTF16("message"),
                        WebKit::WebTextDirectionDefault,
                        UTF8ToUTF16("chrome-test://testing/"),
                        UTF8ToUTF16("REPLACE-ME"),
                        new_delegate);
  }

  Notification CreateRichTestNotification(const std::string& id,
                                          TestDelegate** delegate = NULL) {
    TestDelegate* new_delegate = new TestDelegate(id);
    if (delegate) {
      *delegate = new_delegate;
      new_delegate->AddRef();
    }

    message_center::RichNotificationData data;

    return Notification(message_center::NOTIFICATION_TYPE_BASE_FORMAT,
                        GURL("chrome-test://testing/"),
                        ASCIIToUTF16("title"),
                        ASCIIToUTF16("message"),
                        gfx::Image(),
                        WebKit::WebTextDirectionDefault,
                        UTF8ToUTF16("chrome-test://testing/"),
                        UTF8ToUTF16("REPLACE-ME"),
                        data,
                        new_delegate);
  }
};

// TODO(rsesek): Implement Message Center on Mac and get these tests passing
// for real. http://crbug.com/179904
#if !defined(OS_MACOSX)

IN_PROC_BROWSER_TEST_F(MessageCenterNotificationsTest, RetrieveBaseParts) {
  // Make sure comamnd-line switch has an effect.
  EXPECT_EQ(NotificationUIManager::DelegatesToMessageCenter(),
            message_center::IsRichNotificationEnabled());
  EXPECT_TRUE(manager());
  EXPECT_TRUE(message_center());
}

// MessaceCenter-specific test.
#if defined(RUN_MESSAGE_CENTER_TESTS)
#define MAYBE_BasicAddCancel BasicAddCancel
#else
#define MAYBE_BasicAddCancel DISABLED_BasicAddCancel
#endif

IN_PROC_BROWSER_TEST_F(MessageCenterNotificationsTest, MAYBE_BasicAddCancel) {
  EXPECT_TRUE(NotificationUIManager::DelegatesToMessageCenter());
  manager()->Add(CreateTestNotification("hey"), profile());
  EXPECT_EQ(1u, message_center()->NotificationCount());
  manager()->CancelById("hey");
  EXPECT_EQ(0u, message_center()->NotificationCount());
}

// MessaceCenter-specific test.
#if defined(RUN_MESSAGE_CENTER_TESTS)
#define MAYBE_BasicDelegate BasicDelegate
#else
#define MAYBE_BasicDelegate DISABLED_BasicDelegate
#endif

IN_PROC_BROWSER_TEST_F(MessageCenterNotificationsTest, MAYBE_BasicDelegate) {
  EXPECT_TRUE(NotificationUIManager::DelegatesToMessageCenter());
  TestDelegate* delegate;
  manager()->Add(CreateTestNotification("hey", &delegate), profile());
  // Verify that delegate accumulated correct log of events.
  EXPECT_EQ("Display_", delegate->log());
  manager()->CancelById("hey");
  // Verify that delegate accumulated correct log of events.
  EXPECT_EQ("Display_Close_programmatically_", delegate->log());
  delegate->Release();
}

// MessaceCenter-specific test.
#if defined(RUN_MESSAGE_CENTER_TESTS)
#define MAYBE_ButtonClickedDelegate ButtonClickedDelegate
#else
#define MAYBE_ButtonClickedDelegate DISABLED_ButtonClickedDelegate
#endif

IN_PROC_BROWSER_TEST_F(MessageCenterNotificationsTest,
                       MAYBE_ButtonClickedDelegate) {
  EXPECT_TRUE(NotificationUIManager::DelegatesToMessageCenter());
  TestDelegate* delegate;
  manager()->Add(CreateTestNotification("n", &delegate), profile());
  message_center()->ClickOnNotificationButton("n", 1);
  // Verify that delegate accumulated correct log of events.
  EXPECT_EQ("Display_ButtonClick_1_", delegate->log());
  delegate->Release();
}

// MessaceCenter-specific test.
#if defined(RUN_MESSAGE_CENTER_TESTS)
#define MAYBE_UpdateExistingNotification UpdateExistingNotification
#else
#define MAYBE_UpdateExistingNotification DISABLED_UpdateExistingNotification
#endif

IN_PROC_BROWSER_TEST_F(MessageCenterNotificationsTest,
                       MAYBE_UpdateExistingNotification) {
  EXPECT_TRUE(NotificationUIManager::DelegatesToMessageCenter());
  TestDelegate* delegate;
  manager()->Add(CreateTestNotification("n", &delegate), profile());
  TestDelegate* delegate2;
  manager()->Add(CreateRichTestNotification("n", &delegate2), profile());

  manager()->CancelById("n");
  EXPECT_EQ("Display_Close_programmatically_", delegate->log());
  EXPECT_EQ("Close_programmatically_", delegate2->log());

  delegate->Release();
  delegate2->Release();
}

#endif  // !defined(OS_MACOSX)
