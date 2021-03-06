#include "wsjcpp_employees.h"
#include <algorithm>
#include <wsjcpp_core.h>
#include <iostream>

// ---------------------------------------------------------------------

std::map<std::string, WsjcppEmployBase*> *g_pWsjcppEmployees = nullptr;
std::vector<std::string> *g_pWsjcppInitEmployees = nullptr;
std::vector<std::string> *g_pWsjcppInitWith = nullptr;

// ---------------------------------------------------------------------

void WsjcppEmployees::initGlobalVariables() {
    if (g_pWsjcppEmployees == nullptr) {
        // WsjcppLog::info(std::string(), "Create employees map");
        g_pWsjcppEmployees = new std::map<std::string, WsjcppEmployBase*>();
    }
    if (g_pWsjcppInitEmployees == nullptr) {
        // WsjcppLog::info(std::string(), "Create init employees vector");
        g_pWsjcppInitEmployees = new std::vector<std::string>();
    }
    if (g_pWsjcppInitWith == nullptr) {
        // WsjcppLog::info(std::string(), "Create init employees vector");
        g_pWsjcppInitWith = new std::vector<std::string>();
    }    
}

// ---------------------------------------------------------------------

void WsjcppEmployees::deinitGlobalVariables() {
    const std::string TAG = "WsjcppEmployees::deinit";
    if (g_pWsjcppEmployees != nullptr) {
        std::map<std::string, WsjcppEmployBase*>::iterator it;
        for (it = g_pWsjcppEmployees->begin(); it != g_pWsjcppEmployees->end(); ++it) {
            std::string sEmployName = it->first;
            WsjcppLog::ok(TAG, sEmployName + " ... UNREGISTERED");
            delete it->second;
            it->second = nullptr;
        }
        g_pWsjcppEmployees->clear();
        delete g_pWsjcppEmployees;
        g_pWsjcppEmployees = nullptr;
    }

    if (g_pWsjcppInitEmployees != nullptr) {
        g_pWsjcppInitEmployees->clear();
        delete g_pWsjcppInitEmployees;
        g_pWsjcppInitEmployees = nullptr;
    }

    if (g_pWsjcppInitWith != nullptr) {
        g_pWsjcppInitWith->clear();
        delete g_pWsjcppInitWith;
        g_pWsjcppInitWith = nullptr;
    }
}

// ---------------------------------------------------------------------

void WsjcppEmployees::addEmploy(const std::string &sName, WsjcppEmployBase* pEmploy) {
    WsjcppEmployees::initGlobalVariables();
    if (g_pWsjcppEmployees->find(sName) != g_pWsjcppEmployees->end()) {
        WsjcppLog::err(sName, "Already registered");
    } else {
        g_pWsjcppEmployees->insert(std::pair<std::string, WsjcppEmployBase*>(sName,pEmploy));
        // WsjcppLog::info(sName, "Registered");
    }
}

// ---------------------------------------------------------------------

bool WsjcppEmployees::init(const std::vector<std::string> &vStart) {
    WsjcppEmployees::initGlobalVariables();
    std::string TAG = "WsjcppEmployees::init";

    for (unsigned int i = 0; i < vStart.size(); i++) {
        g_pWsjcppInitEmployees->push_back(vStart[i]);
        g_pWsjcppInitWith->push_back(vStart[i]);
        WsjcppLog::info(TAG, "with " + vStart[i]);
    }
    
    bool bRepeat = true;
    while (bRepeat) {
        bRepeat = false;
        std::map<std::string, WsjcppEmployBase*>::iterator it = g_pWsjcppEmployees->begin();
        for (; it!=g_pWsjcppEmployees->end(); ++it) {
            std::string sEmployName = it->first;
            WsjcppEmployBase *pEmploy = it->second;

            if (std::find(g_pWsjcppInitEmployees->begin(), g_pWsjcppInitEmployees->end(), sEmployName) != g_pWsjcppInitEmployees->end()) {
                continue;
            }

            unsigned int nRequireLoaded = 0;
            for (unsigned int i = 0; i < pEmploy->loadAfter().size(); i++) {
                std::string sRequireEmploy = pEmploy->loadAfter()[i];
                if (std::find(g_pWsjcppInitEmployees->begin(), g_pWsjcppInitEmployees->end(), sRequireEmploy) != g_pWsjcppInitEmployees->end()) {
                    nRequireLoaded++;
                }
            }
            if (pEmploy->loadAfter().size() == nRequireLoaded) {
                if (!pEmploy->init()) {
                    WsjcppLog::err(TAG, sEmployName + " ... INIT_FAIL");
                    return false;
                }
                g_pWsjcppInitEmployees->push_back(sEmployName);
                bRepeat = true;
                WsjcppLog::ok(TAG, sEmployName + " ... INIT_OK");
            }
        }
    }
    return true;
}

// ---------------------------------------------------------------------

bool WsjcppEmployees::deinit() {
    const std::string TAG = "WsjcppEmployees::deinit";
    if (g_pWsjcppInitEmployees == nullptr
        || g_pWsjcppInitWith == nullptr 
        || g_pWsjcppEmployees == nullptr
    ) {
        WsjcppLog::err(TAG, "You must call WsjcppEmployees::init before deinit");
        return false;
    }

    int nInitedCount = g_pWsjcppInitEmployees->size();
    for (int i = nInitedCount-1; i >= 0; i--) {
        std::string sEmployName = g_pWsjcppInitEmployees->at(i);
        if (std::find(g_pWsjcppInitWith->begin(), g_pWsjcppInitWith->end(), sEmployName) != g_pWsjcppInitWith->end()) {
            WsjcppLog::info(TAG,  sEmployName + " ... SKIP_INIT_WITH");
            continue;
        } 

        std::map<std::string, WsjcppEmployBase*>::iterator it;
        it = g_pWsjcppEmployees->find(sEmployName);
        if (it == g_pWsjcppEmployees->end()) {
            WsjcppLog::err(TAG,  sEmployName + " ... DEINIT_NOT_FOUND");
            return false;
        }
        WsjcppEmployBase *pEmploy = it->second;
        if (pEmploy->deinit()) {
            WsjcppLog::ok(TAG, sEmployName + " ... DEINIT_OK");
        } else {
            WsjcppLog::err(TAG,  sEmployName + " ... DEINIT_FAIL");
            return false;
        }
    };

    g_pWsjcppInitEmployees->clear();
    delete g_pWsjcppInitEmployees;
    g_pWsjcppInitEmployees = nullptr;

    g_pWsjcppInitWith->clear();
    delete g_pWsjcppInitWith;
    g_pWsjcppInitWith = nullptr;
    return true;
}


// ---------------------------------------------------------------------

// WsjcppEmployBase
WsjcppEmployBase::WsjcppEmployBase(const std::string &sName, const std::vector<std::string> &vAfter) {
    TAG = sName;
    m_sName = sName;

    for (unsigned int i = 0; i < vAfter.size(); i++) {
        m_vLoadAfter.push_back(vAfter[i]);
    }
    WsjcppEmployees::addEmploy(m_sName, this);
}

// ---------------------------------------------------------------------

WsjcppEmployBase::~WsjcppEmployBase() {
    // nothing
}

// ---------------------------------------------------------------------

const std::vector<std::string> &WsjcppEmployBase::loadAfter() {
    return m_vLoadAfter;
}


// ---------------------------------------------------------------------
// WJSCppEmployRuntimeGlobalCache

REGISTRY_WJSCPP_EMPLOY(WJSCppEmployRuntimeGlobalCache)

// ---------------------------------------------------------------------

WJSCppEmployRuntimeGlobalCache::WJSCppEmployRuntimeGlobalCache()
    : WsjcppEmployBase(WJSCppEmployRuntimeGlobalCache::name(), {}) {

    TAG = WJSCppEmployRuntimeGlobalCache::name();
}

// ---------------------------------------------------------------------

bool WJSCppEmployRuntimeGlobalCache::init() {
    // checking settings
    WsjcppLog::info(TAG, "init");
    return true;
}

// ---------------------------------------------------------------------

bool WJSCppEmployRuntimeGlobalCache::deinit() {
    // checking settings
    WsjcppLog::info(TAG, "deinit");
    m_sStringMap.clear();
    return true;
}

// ---------------------------------------------------------------------

void WJSCppEmployRuntimeGlobalCache::set(const std::string &sName, const std::string &sValue) {
    m_sStringMap[sName] = sValue;
}

// ---------------------------------------------------------------------

bool WJSCppEmployRuntimeGlobalCache::has(const std::string &sName) {
    return m_sStringMap.find(sName) != m_sStringMap.end();
}

// ---------------------------------------------------------------------

std::string WJSCppEmployRuntimeGlobalCache::get(const std::string &sName) {
    if (m_sStringMap.find(sName) != m_sStringMap.end()) {
        return m_sStringMap[sName];
    }
    return "";
}

// ---------------------------------------------------------------------