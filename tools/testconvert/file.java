/*
 * Decompiled with CFR 0_78.
 * 
 * Could not load the following classes:
 *  android.accessibilityservice.AccessibilityServiceInfo
 *  android.content.pm.ResolveInfo
 *  android.os.Build
 *  android.os.Build$VERSION
 *  android.support.v4.accessibilityservice.AccessibilityServiceInfoCompat$AccessibilityServiceInfoIcsImpl
 *  android.support.v4.accessibilityservice.AccessibilityServiceInfoCompat$AccessibilityServiceInfoJellyBeanMr2
 *  android.support.v4.accessibilityservice.AccessibilityServiceInfoCompat$AccessibilityServiceInfoStubImpl
 *  android.support.v4.accessibilityservice.AccessibilityServiceInfoCompat$AccessibilityServiceInfoVersionImpl
 */
package android.support.v4.accessibilityservice;

import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.pm.ResolveInfo;
import android.os.Build;
import android.support.v4.accessibilityservice.AccessibilityServiceInfoCompat;

public class AccessibilityServiceInfoCompat {
    public static final int CAPABILITY_CAN_FILTER_KEY_EVENTS = 8;
    public static final int CAPABILITY_CAN_REQUEST_ENHANCED_WEB_ACCESSIBILITY = 4;
    public static final int CAPABILITY_CAN_REQUEST_TOUCH_EXPLORATION = 2;
    public static final int CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT = 1;
    public static final int DEFAULT = 1;
    public static final int FEEDBACK_ALL_MASK = -1;
    public static final int FEEDBACK_BRAILLE = 32;
    public static final int FLAG_INCLUDE_NOT_IMPORTANT_VIEWS = 2;
    public static final int FLAG_REPORT_VIEW_IDS = 16;
    public static final int FLAG_REQUEST_ENHANCED_WEB_ACCESSIBILITY = 8;
    public static final int FLAG_REQUEST_FILTER_KEY_EVENTS = 32;
    public static final int FLAG_REQUEST_TOUCH_EXPLORATION_MODE = 4;
    private static final AccessibilityServiceInfoVersionImpl IMPL;

    static {
        if (Build.VERSION.SDK_INT >= 18) {
            AccessibilityServiceInfoCompat.IMPL = new AccessibilityServiceInfoJellyBeanMr2();
            return;
        }
        if (Build.VERSION.SDK_INT >= 14) {
            AccessibilityServiceInfoCompat.IMPL = new AccessibilityServiceInfoIcsImpl();
            return;
        }
        AccessibilityServiceInfoCompat.IMPL = new AccessibilityServiceInfoStubImpl();
    }

    private AccessibilityServiceInfoCompat() {
    }

    public static String capabilityToString(int n) {
        switch (n) {
            default: {
                return "UNKNOWN";
            }
            case 1: {
                return "CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT";
            }
            case 2: {
                return "CAPABILITY_CAN_REQUEST_TOUCH_EXPLORATION";
            }
            case 4: {
                return "CAPABILITY_CAN_REQUEST_ENHANCED_WEB_ACCESSIBILITY";
            }
            case 8: 
        }
        return "CAPABILITY_CAN_FILTER_KEY_EVENTS";
    }

    public static String feedbackTypeToString(int n) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("[");
        block7 : while (n > 0) {
            int n2 = 1 << Integer.numberOfTrailingZeros(n);
            n&=~ n2;
            if (stringBuilder.length() > 1) {
                stringBuilder.append(", ");
            }
            switch (n2) {
                default: {
                    continue block7;
                }
                case 1: {
                    stringBuilder.append("FEEDBACK_SPOKEN");
                    continue block7;
                }
                case 4: {
                    stringBuilder.append("FEEDBACK_AUDIBLE");
                    continue block7;
                }
                case 2: {
                    stringBuilder.append("FEEDBACK_HAPTIC");
                    continue block7;
                }
                case 16: {
                    stringBuilder.append("FEEDBACK_GENERIC");
                    continue block7;
                }
                case 8: 
            }
            stringBuilder.append("FEEDBACK_VISUAL");
        }
        stringBuilder.append("]");
        return stringBuilder.toString();
    }

    public static String flagToString(int n) {
        switch (n) {
            default: {
                return null;
            }
            case 1: {
                return "DEFAULT";
            }
            case 2: {
                return "FLAG_INCLUDE_NOT_IMPORTANT_VIEWS";
            }
            case 4: {
                return "FLAG_REQUEST_TOUCH_EXPLORATION_MODE";
            }
            case 8: {
                return "FLAG_REQUEST_ENHANCED_WEB_ACCESSIBILITY";
            }
            case 16: {
                return "FLAG_REPORT_VIEW_IDS";
            }
            case 32: 
        }
        return "FLAG_REQUEST_FILTER_KEY_EVENTS";
    }

    public static boolean getCanRetrieveWindowContent(AccessibilityServiceInfo accessibilityServiceInfo) {
        return AccessibilityServiceInfoCompat.IMPL.getCanRetrieveWindowContent(accessibilityServiceInfo);
    }

    public static int getCapabilities(AccessibilityServiceInfo accessibilityServiceInfo) {
        return AccessibilityServiceInfoCompat.IMPL.getCapabilities(accessibilityServiceInfo);
    }

    public static String getDescription(AccessibilityServiceInfo accessibilityServiceInfo) {
        return AccessibilityServiceInfoCompat.IMPL.getDescription(accessibilityServiceInfo);
    }

    public static String getId(AccessibilityServiceInfo accessibilityServiceInfo) {
        return AccessibilityServiceInfoCompat.IMPL.getId(accessibilityServiceInfo);
    }

    public static ResolveInfo getResolveInfo(AccessibilityServiceInfo accessibilityServiceInfo) {
        return AccessibilityServiceInfoCompat.IMPL.getResolveInfo(accessibilityServiceInfo);
    }

    public static String getSettingsActivityName(AccessibilityServiceInfo accessibilityServiceInfo) {
        return AccessibilityServiceInfoCompat.IMPL.getSettingsActivityName(accessibilityServiceInfo);
    }
}
