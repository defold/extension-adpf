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
	jmethodID      m_Initialize;
	jmethodID      m_Finalize;
	jmethodID      m_ReportWorkDuration;
	jmethodID      m_UpdateTargetFPS;
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

static int PerformanceHint_Init(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	uint64_t target_fps_nanos = (uint64_t)luaL_checknumber(L, 1);
	dmLogInfo("PerformanceHint_Init %llu", target_fps_nanos);
	CallVoidMethodLong(g_ADPF.m_JNI, g_ADPF.m_Initialize, target_fps_nanos);
	return 0;
}

static int PerformanceHint_UpdateTargetFPS(lua_State* L) {
	DM_LUA_STACK_CHECK(L, 0);
	uint64_t target_fps_nanos = (uint64_t)luaL_checknumber(L, 1);
	dmLogInfo("PerformanceHint_UpdateTargetFPS %llu", target_fps_nanos);
	CallVoidMethodLong(g_ADPF.m_JNI, g_ADPF.m_UpdateTargetFPS, target_fps_nanos);
	return 0;
}


static const luaL_reg PerformanceHint_methods[] =
{
	{"init", PerformanceHint_Init},
	{"update_target_fps", PerformanceHint_UpdateTargetFPS},
	{0, 0}
};

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
	g_ADPF.m_Initialize 		= env->GetMethodID(cls, "initialize", "(J)V");
	g_ADPF.m_Finalize 			= env->GetMethodID(cls, "finalize", "()V");
	g_ADPF.m_ReportWorkDuration = env->GetMethodID(cls, "reportWorkDuration", "()V");
	g_ADPF.m_UpdateTargetFPS 	= env->GetMethodID(cls, "updateTargetFPS", "(J)V");
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
	CallVoidMethod(g_ADPF.m_JNI, g_ADPF.m_Finalize);
#endif
	return dmExtension::RESULT_OK;
}

// https://developer.android.com/ndk/reference/group/a-performance-hint
dmExtension::Result UpdateADPFExtension(dmExtension::Params* params) {
#if defined(DM_PLATFORM_ANDROID)
	CallVoidMethod(g_ADPF.m_JNI, g_ADPF.m_ReportWorkDuration);
#endif
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(ADPF, LIB_NAME, AppInitializeADPFExtension, AppFinalizeADPFExtension, InitializeADPFExtension, UpdateADPFExtension, 0, FinalizeADPFExtension)
