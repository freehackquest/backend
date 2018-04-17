#include <employ_orchestra.h>
#include <employ_settings.h>

REGISTRY_EMPLOY(EmployOrchestra)

// ---------------------------------------------------------------------

#include <log.h>

#include <QTextStream>
#include <QFile>
#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QStringList>

// ---------------------------------------------------------------------

void EmployOrchestra::test(){

}


// ---------------------------------------------------------------------

void EmployOrchestra::initSettings(){
    TAG = "EmployOrchestra";

    Log::info(TAG, "Start init settings");

    EmploySettings *pSettings = findEmploy<EmploySettings>();
    path_dir_lxc_ssl = pSettings->getSettString("path_dir_lxc_ssl").toStdString();
    std::string lxd_server_ip = pSettings->getSettString("lxd_server_ip").toStdString();
    std::string lxd_server_port = pSettings->getSettString("lxd_server_port").toStdString();
    lxd_address = "https://" + lxd_server_ip + ":" + lxd_server_port;
    QString lxd_passwd = pSettings->getSettString("lxd_passwd");


    //TO DO
    //Connect to LXD in first time for add SSL cert
    //


    //TO DO
    //Pull existing containers

}

bool EmployOrchestra::create_container(std::string name, std::string &error){
    Log::info(TAG, "Starting creation container " + name);
    LXDContainer container = LXDContainer(name);

    if(container.create()){
        Log::info(TAG, "Created container " + name);
    }

    if(!(container.get_error() == "")){
        error = container.get_error();
        return false;
    }

    containers_map.insert(std::make_pair(name, &container));
    return true;
}

LXDContainer * EmployOrchestra::get_container(std::string name){
    LXDContainer * container = containers_map[name];
    return container;
}

bool EmployOrchestra::find_container(std::string name){
    if (containers_map.find(name) == containers_map.end())
        return false;

    return true;
}

// ---------------------------------------------------------------------

static size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
  ((std::string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

static char errorBuffer[CURL_ERROR_SIZE];

bool EmployOrchestra::send_post_request(std::string address, std::string settings, std::string &response, std::string & error){
    CURLcode ret;
    CURL *hnd;

    hnd = curl_easy_init();
    std::string hostname = lxd_address + address;
    curl_easy_setopt(hnd, CURLOPT_URL, hostname.c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, settings.c_str());
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)settings.size());
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    std::string ssl_crt = path_dir_lxc_ssl + "/client.crt";
    curl_easy_setopt(hnd, CURLOPT_SSLCERT, ssl_crt.c_str());
    std::string ssl_key = path_dir_lxc_ssl + "/client.key";
    curl_easy_setopt(hnd, CURLOPT_SSLKEY, ssl_key.c_str());
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    //curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
    //Saving response
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errorBuffer);
    ret = curl_easy_perform(hnd);

    if(ret != CURLE_OK) {
        Log::err(TAG, " Failed send POST request with error " + std::string(errorBuffer));
        error = errorBuffer;
        return false;
    }

    curl_easy_cleanup(hnd);
    hnd = NULL;

    return true;
}

bool EmployOrchestra::send_put_request(std::string address, std::string settings, std::string &response, std::string & error){
    CURLcode ret;
    CURL *hnd;

    hnd = curl_easy_init();
    std::string hostname = lxd_address + address;
    curl_easy_setopt(hnd, CURLOPT_URL, hostname.c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, settings.c_str());
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)settings.size());
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    std::string ssl_crt = path_dir_lxc_ssl + "/client.crt";
    curl_easy_setopt(hnd, CURLOPT_SSLCERT, ssl_crt.c_str());
    std::string ssl_key = path_dir_lxc_ssl + "/client.key";
    curl_easy_setopt(hnd, CURLOPT_SSLKEY, ssl_key.c_str());
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
    //Saving response
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errorBuffer);
    ret = curl_easy_perform(hnd);

    if(ret != CURLE_OK) {
        Log::err(TAG, "Failed send POST request " + std::string(errorBuffer));
        error = std::string(errorBuffer);
        return false;
    }

    curl_easy_cleanup(hnd);
    hnd = NULL;

    return true;
}

bool EmployOrchestra::send_get_request(std::string address, std::string &response, std::string & error){

    CURLcode ret;
    CURL *hnd;

    hnd = curl_easy_init();
    std::string hostname = lxd_address + address;
    curl_easy_setopt(hnd, CURLOPT_URL, hostname.c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    std::string ssl_crt = path_dir_lxc_ssl + "/client.crt";
    curl_easy_setopt(hnd, CURLOPT_SSLCERT, ssl_crt.c_str());
    std::string ssl_key = path_dir_lxc_ssl + "/client.key";
    curl_easy_setopt(hnd, CURLOPT_SSLKEY, ssl_key.c_str());
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    //curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
    //Saving response
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errorBuffer);
    ret = curl_easy_perform(hnd);

    if(ret != CURLE_OK) {
        Log::err(TAG, "Failed send POST request " + std::string(errorBuffer));
        error = std::string(errorBuffer);
        return false;
    }

    curl_easy_cleanup(hnd);
    hnd = NULL;

    return true;
}

bool EmployOrchestra::pull_container_names(){
    CURLcode ret;
    CURL *hnd;
    std::string readBuffer;

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_URL, "https://127.0.0.1:8443/1.0/containers");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_SSLCERT, "/home/sergo/.config/lxc/client.crt");
    curl_easy_setopt(hnd, CURLOPT_SSLKEY, "/home/sergo/.config/lxc/client.key");
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    //Saving response
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errorBuffer);

    ret = curl_easy_perform(hnd);

    if(ret != CURLE_OK) {
        fprintf(stderr, "Failed to listing container [%s]\n", errorBuffer);
        return false;
    }

    QJsonDocument response = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());

    if(response.isObject()){
        QJsonObject res = response.object();
        if(res.contains("metadata")){
            QJsonArray metadata = res.value("metadata").toArray();

            foreach (const QJsonValue & value, metadata) {
                names.push_back(value.toString().toStdString());
            }
        }
    }


    curl_easy_cleanup(hnd);
    hnd = NULL;
    return true;
}

// ---------------------------------------------------------------------

