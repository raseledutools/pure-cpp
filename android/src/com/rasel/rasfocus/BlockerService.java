package com.rasel.rasfocus;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.Intent;
import android.util.Log;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import java.util.Arrays;
import java.util.List;

public class BlockerService extends AccessibilityService {

    // তোমার হার্ডকোর ব্লক লিস্ট (বাংলা, বাংলিশ, ইংরেজি)
    private final List<String> badKeywords = Arrays.asList(
        "porn", "xxx", "sex", "nude", "xvideos", "pornhub", 
        "choti", "magi", "boccha", "adult video" // তোমার ইচ্ছেমতো আরও যোগ করতে পারো
    );

    // যে অ্যাপগুলোতে ঢোকা একদম নিষেধ (Settings & Uninstall block)
    private final List<String> blockedPackages = Arrays.asList(
        "com.android.settings",               // সেটিংস ব্লক
        "com.android.packageinstaller",       // আনইনস্টল ব্লক
        "com.google.android.packageinstaller" // গুগল প্যাকেজ ইন্সটলার
    );

    @Override
    protected void onServiceConnected() {
        // সার্ভিসটি চালু হওয়ার সাথে সাথে কনফিগারেশন সেট করবে
        AccessibilityServiceInfo info = new AccessibilityServiceInfo();
        info.eventTypes = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED;
        info.feedbackType = AccessibilityServiceInfo.FEEDBACK_GENERIC;
        info.flags = AccessibilityServiceInfo.FLAG_INCLUDE_NOT_IMPORTANT_VIEWS;
        setServiceInfo(info);
        Log.d("RasFocus", "Hardcore Protection Activated!");
    }

    @Override
    public void onAccessibilityEvent(AccessibilityEvent event) {
        if (event == null) return;

        String packageName = event.getPackageName() != null ? event.getPackageName().toString() : "";
        
        // ১. আনইনস্টল বা সেটিংস ব্লক লজিক
        if (blockedPackages.contains(packageName)) {
            performGlobalAction(GLOBAL_ACTION_HOME); // সাথে সাথে হোম স্ক্রিনে পাঠিয়ে দেবে
            Log.d("RasFocus", "Settings or Uninstall Blocked!");
            return;
        }

        // ২. হোয়াটসঅ্যাপ, ফেসবুক বা যেকোনো স্ক্রিনের টেক্সট রিডিং (AI Keyword Filter)
        AccessibilityNodeInfo source = event.getSource();
        if (source != null) {
            checkAndBlockText(source);
        }
    }

    // স্ক্রিনের প্রতিটি লেখা পড়ার রিকার্সিভ ফাংশন
    private void checkAndBlockText(AccessibilityNodeInfo node) {
        if (node == null) return;

        if (node.getText() != null) {
            String screenText = node.getText().toString().toLowerCase();
            
            // কিওয়ার্ড ম্যাচিং
            for (String keyword : badKeywords) {
                if (screenText.contains(keyword)) {
                    Log.d("RasFocus", "Adult Content Detected: " + keyword);
                    
                    // অ্যাকশন: অটোমেটিক ব্যাক বাটন চাপবে বা হোম স্ক্রিনে পাঠাবে
                    performGlobalAction(GLOBAL_ACTION_BACK); 
                    
                    // তুমি চাইলে এখানে C++ কে সিগন্যাল পাঠাতে পারো ওভারলে দেখানোর জন্য
                    return; 
                }
            }
        }

        for (int i = 0; i < node.getChildCount(); i++) {
            checkAndBlockText(node.getChild(i));
        }
    }

    @Override
    public void onInterrupt() {
        // সার্ভিস বন্ধ হলে কল হয়
    }
}
