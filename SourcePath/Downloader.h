#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

struct DownloadFile {
    std::string url;
    std::string localPath;
    std::string displayName;
    bool completed;
    bool failed;
    float progress;
    std::string errorMessage;
};

class CDownloader {
public:
    CDownloader();
    ~CDownloader();

    void AddFileToQueue(const std::string& url, const std::string& localPath, const std::string& displayName);
    void StartDownloads();
    void CancelDownloads();
    
    bool IsDownloading() const;
    float GetTotalProgress() const;
    int GetCompletedCount() const;
    int GetTotalCount() const;
    
    std::vector<DownloadFile> GetDownloadQueue() const;
    
    void SetProgressCallback(std::function<void(float, const std::string&)> callback);

private:
    void DownloadThread();
    bool PerformDownload(DownloadFile& file);
    void UpdateProgress(float progress, const std::string& status);

private:
    std::vector<DownloadFile> m_downloadQueue;
    std::thread m_downloadThread;
    std::atomic<bool> m_isDownloading;
    std::atomic<bool> m_shouldCancel;
    mutable std::mutex m_queueMutex;
    
    std::function<void(float, const std::string&)> m_progressCallback;
};
