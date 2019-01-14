#pragma once
#include "base/ccConfig.h"
#if USE_SPINE > 0

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"

extern se::Object* __jsb_spine_SpineRenderer_proto;
extern se::Class* __jsb_spine_SpineRenderer_class;

bool js_register_spine_SpineRenderer(se::Object* obj);
bool register_all_cocos2dx_spine(se::Object* obj);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setUseTint);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setTimeScale);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_paused);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setAttachment);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setBonesToSetupPose);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setSlotsToSetupPose);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_stopSchedule);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_isOpacityModifyRGB);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setDebugSlotsEnabled);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_getMaterialData);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_initWithJsonFile);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setToSetupPose);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setOpacityModifyRGB);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_initWithBinaryFile);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_onEnable);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_beginSchedule);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_getDebugData);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_initWithSkeleton);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_update);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_getAttachment);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_initialize);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setDebugBonesEnabled);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_getTimeScale);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setSlotsRange);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_onDisable);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setColor);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_setSkin);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_findSlot);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_updateWorldTransform);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_getSkeleton);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_findBone);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineRenderer_SpineRenderer);

extern se::Object* __jsb_spine_SpineAnimation_proto;
extern se::Class* __jsb_spine_SpineAnimation_class;

bool js_register_spine_SpineAnimation(se::Object* obj);
bool register_all_cocos2dx_spine(se::Object* obj);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setAnimation);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_findAnimation);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setMix);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_addEmptyAnimation);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setDisposeListener);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setAnimationStateData);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setEndListener);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_getState);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setCompleteListener);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_getCurrent);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setEventListener);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setEmptyAnimation);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_clearTrack);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setInterruptListener);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_addAnimation);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setEmptyAnimations);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_clearTracks);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_setStartListener);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_createWithBinaryFile);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_create);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_createWithJsonFile);
SE_DECLARE_FUNC(js_cocos2dx_spine_SpineAnimation_SpineAnimation);

#endif //#if USE_SPINE > 0