#include "msParam.h"
#include "irods_ms_plugin.hpp"

#include <map>

namespace {
    typedef void (*FreeingFunction)(msParam_t*);

    struct CompareCStrings {
        bool operator()(const char* l, const char* r) const {
            return std::strcmp(l, r) < 0;
        }
    };

    typedef std::map<const char*, FreeingFunction, CompareCStrings> Map;

    void
    free_ExecCmdOut_MS_T(msParam_t* inpParam) {
        freeCmdExecOut(static_cast<execCmdOut_t*>(inpParam->inOutStruct));
        inpParam->inOutStruct = NULL;
    }

    Map
    create_map() {
        Map m;
        m[ExecCmdOut_MS_T] = free_ExecCmdOut_MS_T;
        return m;
    }

    const Map type_to_freeing_function = create_map();
}

int
msifree_microservice_out(msParam_t *inpParam, ruleExecInfo_t *rei) {
    if (inpParam == NULL) {
        rodsLog(LOG_NOTICE, "msifree_microservice_out: NULL msParam_t");
        return 0;
    }

    if (inpParam->type == NULL) {
        rodsLog(LOG_NOTICE, "msifree_microservice_out: NULL msParam_t.type");
        return 0;
    }

    auto it = type_to_freeing_function.find(inpParam->type);
    if (it == type_to_freeing_function.end()) {
        rodsLog(LOG_ERROR, "msifree_microservice_out: not implemented for type: %s", inpParam->type);
        return USER_PARAM_TYPE_ERR;
    }

    it->second(inpParam);

    free(inpParam->type);
    inpParam->type = NULL;

    free(inpParam->label);
    inpParam->label = NULL;

    return 0;
}

extern "C"
irods::ms_table_entry* plugin_factory(void) {
    irods::ms_table_entry *msvc = new irods::ms_table_entry(1);
    msvc->add_operation<msParam_t*, ruleExecInfo_t*>("msifree_microservice_out", std::function<int(msParam_t*, ruleExecInfo_t*)>(msifree_microservice_out));
    return msvc;
}
