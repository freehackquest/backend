#include <notify_to_all_task.h>
#include <employees.h>
#include <fallen.h>

NotifyToAllTask::NotifyToAllTask(const nlohmann::json &jsonMessage) {
    TAG = "NotifyToAllTask";
    m_jsonMessage = jsonMessage;
}

NotifyToAllTask::~NotifyToAllTask() {

}

void NotifyToAllTask::run() {
    WSJCppLog::info(TAG, "send notify");
    EmployServer *pServer = findWSJCppEmploy<EmployServer>();
    pServer->sendToAll(m_jsonMessage);
}
