#define LIB_NAME "ADPF"
#define MODULE_NAME "adpf"
#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID)

#include <dmsdk/dlib/android.h>


struct ADPFJNI
{
	jobject        m_JNI;
	jmethodID      m_HintInitialize;
	jmethodID      m_HintFinalize;
	jmethodID      m_HintReportWorkDuration;
	jmethodID      m_HintUpdateTargetFPS;
	jmethodID      m_ThermalInitialize;
	jmethodID      m_ThermalFinalize;
	jmethodID      m_ThermalGetStatus;
	jmethodID      m_ThermalGetHeadroom;
	bool           m_HintAvailable;
	bool           m_ThermalAvailable;
};

static ADPFJNI g_ADPF;


static void CallVoidMethod(jobject instance, jmethodID method)
{
	dmAndroid::ThreadAttacher threadAttacher;
	JNIEnv* env = threadAttacher.GetEnv();
	env->CallVoidMethod(instance, method);
}

static void CallVoidMethodLong(jobject instance, jmethodID method, const uint64_t arg1)
{
	dmAndroid::ThreadAttacher threadAttacher;
	JNIEnv* env = threadAttacher.GetEnv();
	env->CallVoidMethod(instance, method, arg1);
}

static int CallIntMethodLong(jobject instance, jmethodID method, const uint64_t arg1)
{
	dmAndroid::ThreadAttacher threadAttacher;
	JNIEnv* env = threadAttacher.GetEnv();
	jint result = env->CallIntMethod(instance, method, arg1);
	return result;
}

static int CallIntMethodVoid(jobject instance, jmethodID method)
{
	dmAndroid::ThreadAttacher threadAttacher;
	JNIEnv* env = threadAttacher.GetEnv();
	jint result = env->CallIntMethod(instance, method);
	return result;
}

static float CallFloatMethodInt(jobject instance, jmethodID method, const int arg1)
{
	dmAndroid::ThreadAttacher threadAttacher;
	JNIEnv* env = threadAttacher.GetEnv();
	jfloat result = env->CallFloatMethod(instance, method, arg1);
	return result;
}

/**********************************
 * PerformanceHintAPI
 **********************************/

static int PerformanceHint_Init(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	uint64_t target_fps_nanos = (uint64_t)luaL_checknumber(L, 1);
	dmLogInfo("PerformanceHint_Init %lu", target_fps_nanos);
	jint available = CallIntMethodLong(g_ADPF.m_JNI, g_ADPF.m_HintInitialize, target_fps_nanos);
	lua_pushboolean(L, available);
	g_ADPF.m_HintAvailable = available;
	return 1;
}

static int PerformanceHint_UpdateTargetFPS(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	uint64_t target_fps_nanos = (uint64_t)luaL_checknumber(L, 1);
	dmLogInfo("PerformanceHint_UpdateTargetFPS %lu", target_fps_nanos);
	CallVoidMethodLong(g_ADPF.m_JNI, g_ADPF.m_HintUpdateTargetFPS, target_fps_nanos);
	return 0;
}

static const luaL_reg PerformanceHint_methods[] =
{
	{"init", PerformanceHint_Init},
	{"update_target_fps", PerformanceHint_UpdateTargetFPS},
	{0, 0}
};


/**********************************
 * ThermalAPI
 **********************************/

static int Thermal_Init(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	dmLogInfo("Thermal_Init");
	int available = CallIntMethodVoid(g_ADPF.m_JNI, g_ADPF.m_ThermalInitialize);
	lua_pushnumber(L, available);
	g_ADPF.m_ThermalAvailable = available;
	return 1;
}

static int Thermal_GetHeadroom(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	int forecaseSeconds = luaL_checknumber(L, 1);
	dmLogInfo("Thermal_GetHeadroom %d", forecaseSeconds);
	float headroom = CallFloatMethodInt(g_ADPF.m_JNI, g_ADPF.m_ThermalGetHeadroom, forecaseSeconds);
	lua_pushnumber(L, headroom);
	return 1;
}

static int Thermal_GetStatus(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 1);
	dmLogInfo("Thermal_GetStatus");
	int status = CallIntMethodVoid(g_ADPF.m_JNI, g_ADPF.m_ThermalGetStatus);
	lua_pushnumber(L, status);
	return 1;
}

static const luaL_reg Thermal_methods[] =
{
	{"init", Thermal_Init},
	{"get_headroom", Thermal_GetHeadroom},
	{"get_status", Thermal_GetStatus},
	{0, 0}
};




static void lua_setfieldstringint(lua_State* L, const char* key, uint32_t value)
{
    int top = lua_gettop(L);
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
    assert(top == lua_gettop(L));
}

static void LuaInit(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);

	// get or create adpf global table
	lua_getglobal(L, "adpf");
	if (lua_isnil(L, lua_gettop(L)))
	{
		lua_pushstring(L, "adpf");
		lua_newtable(L);
		lua_settable(L, LUA_GLOBALSINDEX);
		lua_pop(L, 1);
		lua_getglobal(L, "adpf");
	}

	// create hint subtable
	lua_pushstring(L, "hint");
	lua_newtable(L);
	luaL_register(L, NULL, PerformanceHint_methods);
	lua_settable(L, -3);

	// create thermal subtable
	lua_pushstring(L, "thermal");
	lua_newtable(L);
	luaL_register(L, NULL, Thermal_methods);
	lua_setfieldstringint(L, "THERMAL_STATUS_NONE", 0);
	lua_setfieldstringint(L, "THERMAL_STATUS_LIGHT", 1);
	lua_setfieldstringint(L, "THERMAL_STATUS_MODERATE", 2);
	lua_setfieldstringint(L, "THERMAL_STATUS_SEVERE", 3);
	lua_setfieldstringint(L, "THERMAL_STATUS_CRITICAL", 4);
	lua_setfieldstringint(L, "THERMAL_STATUS_EMERGENCY", 5);
	lua_setfieldstringint(L, "THERMAL_STATUS_SHUTDOWN", 6);
	lua_settable(L, -3);

	lua_pop(L, 1); // pop "adpf" global table
}

static void JavaInit() {
	dmAndroid::ThreadAttacher threadAttacher;
	JNIEnv* env = threadAttacher.GetEnv();
	jclass cls = dmAndroid::LoadClass(env, "com.defold.adpf.ADPFJNI");

	// create instance of ADPFJNI class
	jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");
	g_ADPF.m_JNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, threadAttacher.GetActivity()->clazz));

	// get function references
	g_ADPF.m_HintInitialize 		= env->GetMethodID(cls, "hintInitialize", "(J)I");
	g_ADPF.m_HintFinalize 			= env->GetMethodID(cls, "hintFinalize", "()V");
	g_ADPF.m_HintReportWorkDuration = env->GetMethodID(cls, "hintReportWorkDuration", "()V");
	g_ADPF.m_HintUpdateTargetFPS 	= env->GetMethodID(cls, "hintUpdateTargetFPS", "(J)V");

	g_ADPF.m_ThermalInitialize 		= env->GetMethodID(cls, "thermalInitialize", "()I");
	g_ADPF.m_ThermalFinalize 		= env->GetMethodID(cls, "thermalFinalize", "()V");
	g_ADPF.m_ThermalGetHeadroom 	= env->GetMethodID(cls, "thermalGetHeadroom", "(I)F");
	g_ADPF.m_ThermalGetStatus	 	= env->GetMethodID(cls, "thermalGetStatus", "()I");
}

#endif


dmExtension::Result AppInitializeADPFExtension(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeADPFExtension(dmExtension::Params* params) {
#if !defined(DM_PLATFORM_ANDROID)
	dmLogInfo("Extension %s is not supported", LIB_NAME);
#else
	LuaInit(params->m_L);
	JavaInit();
#endif
	return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeADPFExtension(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeADPFExtension(dmExtension::Params* params) {
#if defined(DM_PLATFORM_ANDROID)
	CallVoidMethod(g_ADPF.m_JNI, g_ADPF.m_HintFinalize);
	CallVoidMethod(g_ADPF.m_JNI, g_ADPF.m_ThermalFinalize);
	g_ADPF.m_HintAvailable = 0;
	g_ADPF.m_ThermalAvailable = 0;
#endif
	return dmExtension::RESULT_OK;
}

// https://developer.android.com/ndk/reference/group/a-performance-hint
dmExtension::Result UpdateADPFExtension(dmExtension::Params* params) {
#if defined(DM_PLATFORM_ANDROID)
	if (g_ADPF.m_HintAvailable)
	{
		CallVoidMethod(g_ADPF.m_JNI, g_ADPF.m_HintReportWorkDuration);
	}
#endif
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(ADPF, LIB_NAME, AppInitializeADPFExtension, AppFinalizeADPFExtension, InitializeADPFExtension, UpdateADPFExtension, 0, FinalizeADPFExtension)
