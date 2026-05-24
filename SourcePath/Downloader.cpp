#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Downloader.h"
#include <fstream>
#include <shlobj.h>

CDownloader::CDownloader()
    : m_isDownloading(false)
    , m_shouldCancel(false)
{
}

CDownloader::~CDownloader() {
    CancelDownloads();
    if (m_downloadThread.joinable()) {
        m_downloadThread.join();
    }
}

void CDownloader::AddFileToQueue(const std::string& url, const std::string& localPath, const std::string& displayName) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    DownloadFile file;
    file.url = url;
    file.localPath = localPath;
    file.displayName = displayName;
    file.completed = false;
    file.failed = false;
    file.progress = 0.0f;
    m_downloadQueue.push_back(file);
}

void CDownloader::StartDownloads() {
    if (m_isDownloading) return;
    
    m_shouldCancel = false;
    m_isDownloading = true;
    
    m_downloadThread = std::thread(&CDownloader::DownloadThread, this);
}

void CDownloader::CancelDownloads() {
    m_shouldCancel = true;
    if (m_downloadThread.joinable()) {
        m_downloadThread.join();
    }
    m_isDownloading = false;
}

bool CDownloader::IsDownloading() const {
    return m_isDownloading;
}

float CDownloader::GetTotalProgress() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_downloadQueue.empty()) return 0.0f;
    
    float totalProgress = 0.0f;
    for (const auto& file : m_downloadQueue) {
        totalProgress += file.progress;
    }
    return totalProgress / m_downloadQueue.size();
}

int CDownloader::GetCompletedCount() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    int count = 0;
    for (const auto& file : m_downloadQueue) {
        if (file.completed) count++;
    }
    return count;
}

int CDownloader::GetTotalCount() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_downloadQueue.size();
}

std::vector<DownloadFile> CDownloader::GetDownloadQueue() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_downloadQueue;
}

void CDownloader::SetProgressCallback(std::function<void(float, const std::string&)> callback) {
    m_progressCallback = callback;
}

void CDownloader::DownloadThread() {
    std::vector<DownloadFile> queue;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        queue = m_downloadQueue;
    }
    
    for (size_t i = 0; i < queue.size(); i++) {
        if (m_shouldCancel) break;
        
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_downloadQueue[i].progress = 0.0f;
        }
        
        bool success = PerformDownload(queue[i]);
        
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_downloadQueue[i].completed = success;
            m_downloadQueue[i].failed = !success;
            m_downloadQueue[i].progress = success ? 100.0f : 0.0f;
        }
        
        if (m_progressCallback) {
            float totalProgress = GetTotalProgress();
            std::string status = success ? 
                "Downloaded: " + queue[i].displayName : 
                "Failed: " + queue[i].displayName;
            m_progressCallback(totalProgress, status);
        }
    }
    
    m_isDownloading = false;
}

bool CDownloader::PerformDownload(DownloadFile& downloadFile) {
    HINTERNET hInternet = InternetOpen(L"PClient Downloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;
    
    std::wstring wUrl(downloadFile.url.begin(), downloadFile.url.end());
    HINTERNET hUrl = InternetOpenUrl(hInternet, wUrl.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return false;
    }
    
    // Get file size
    DWORD fileSize = 0;
    DWORD cbSize = sizeof(fileSize);
    HttpQueryInfo(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &fileSize, &cbSize, NULL);
    
    // Create directory if needed
    size_t lastSlash = downloadFile.localPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        std::string dir = downloadFile.localPath.substr(0, lastSlash);
        SHCreateDirectoryEx(NULL, std::wstring(dir.begin(), dir.end()).c_str(), NULL);
    }
    
    std::string outputPath = downloadFile.localPath;
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }
    
    const DWORD chunkSize = 8192;
    BYTE buffer[chunkSize];
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    
    while (InternetReadFile(hUrl, buffer, chunkSize, &bytesRead) && bytesRead > 0) {
        if (m_shouldCancel) {
            outFile.close();
            InternetCloseHandle(hUrl);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        outFile.write((char*)buffer, bytesRead);
        totalBytesRead += bytesRead;
        
        // Update progress
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (fileSize > 0) {
                downloadFile.progress = (float)totalBytesRead / fileSize * 100.0f;
            } else {
                downloadFile.progress = 50.0f; // Unknown size, show 50%
            }
        }
        
        if (m_progressCallback) {
            float totalProgress = GetTotalProgress();
            std::string status = "Downloading: " + downloadFile.displayName;
            m_progressCallback(totalProgress, status);
        }
    }
    
    outFile.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    return true;
}

void CDownloader::UpdateProgress(float progress, const std::string& status) {
    if (m_progressCallback) {
        m_progressCallback(progress, status);
    }
}
