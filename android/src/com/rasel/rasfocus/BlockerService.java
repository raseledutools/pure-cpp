package com.rasel.rasfocus;

import android.accessibilityservice.AccessibilityService;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.content.Intent;
import android.util.Log;

public class BlockerService extends AccessibilityService {

    // তোমার ব্ল্যাকলিস্ট করা কিওয়ার্ডগুলো (এখানে আরও যোগ করতে পারো)
    private final String[] badWords = {
        "sex", "porn", "xvideo", "choti", "xnxx", "mia khalifa", "brazzers", "xxx"
    };

    // তোমার ব্ল্যাকলিস্ট করা অ্যাপের প্যাকেজ নাম (উদাহরণস্বরূপ)
    private final String[] blockedApps = {
        "com.facebook.katana",      // Facebook
        "com.instagram.android",    // Instagram
        "com.zhiliaoapp.musically"  // TikTok
    };

    @Override
    public void onAccessibilityEvent(AccessibilityEvent event) {
        if (event == null) return;

        String packageName = event.getPackageName() != null ? event.getPackageName().toString() : "";

        // ১. Anti-Uninstall & Settings Block (সবচেয়ে কড়া গার্ড)
        // কেউ যদি সেটিংস বা প্লে স্টোর থেকে অ্যাপ ডিলিট করতে চায়, তাকে কিক করবে
        if (packageName.equals("com.android.settings") || 
            packageName.equals("com.android.vending") || 
            packageName.equals("com.google.android.packageinstaller")) {
            
            triggerHardcoreBlock();
            return;
        }

        // ২. Social Media / App Block 
        for (String app : blockedApps) {
            if (packageName.equals(app)) {
                triggerHardcoreBlock();
                return;
            }
        }

        // ৩. AI Keyword Scanner (স্ক্রিনের যেকোনো জায়গায় খারাপ শব্দ খুঁজবে)
        AccessibilityNodeInfo nodeInfo = event.getSource();
        if (nodeInfo != null) {
            if (scanAndLock(nodeInfo)) {
                triggerHardcoreBlock();
            }
        }
    }

    // স্ক্রিনের প্রতিটি লেখা স্ক্যান করার রিকার্সিভ ফাংশন
    private boolean scanAndLock(AccessibilityNodeInfo node) {
        if (node == null) return false;

        // যদি টেক্সট বক্সে বা ব্রাউজারে কিছু লেখা থাকে
        if (node.getText() != null) {
            String text = node.getText().toString().toLowerCase();
            
            for (String word : badWords) {
                // যদি খারাপ কোনো শব্দ ম্যাচ করে যায়
                if (text.contains(word)) {
                    Log.d("RasFocus", "Blocked keyword found: " + word);
                    return true; 
                }
            }
        }

        // স্ক্রিনের ভেতরের অন্যান্য লেআউটগুলো চেক করবে
        for (int i = 0; i < node.getChildCount(); i++) {
            if (scanAndLock(node.getChild(i))) {
                return true;
            }
        }
        return false;
    }

    // যখনই রুলস ব্রেক হবে, এই ফাংশন ফায়ার হবে!
    private void triggerHardcoreBlock() {
        // ১. ব্যাক বাটন প্রেস করবে
        performGlobalAction(GLOBAL_ACTION_BACK);
        // ২. হোম স্ক্রিনে পাঠিয়ে দেবে
        performGlobalAction(GLOBAL_ACTION_HOME);

        // ৩. সাথে সাথে তোমার RasFocus+ অ্যাপটি স্ক্রিনের সামনে ওপেন করে দেবে!
        Intent intent = getPackageManager().getLaunchIntentForPackage("com.rasel.rasfocus");
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(intent);
        }
    }

    @Override
    public void onInterrupt() {
        // সার্ভিস ক্র্যাশ করলে বা ইন্টারাপ্ট হলে এটি কল হয় (ফাঁকাই থাকে)
    }
    
    @Override
    protected void onServiceConnected() {
        super.onServiceConnected();
        Log.d("RasFocus", "Ultimate Blocker Service Connected & Active!");
    }
}
