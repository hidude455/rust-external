/*
 * Comprehensive Error Handler for Rust Anti-Cheat Evasion System
 * Provides robust error handling, logging, and recovery mechanisms
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <DbgHelp.h>

namespace ErrorHandling {
    
    // Error severity levels
    enum class ErrorSeverity {
        Info,
        Warning,
        Error,
        Critical,
        Fatal
    };
    
    // Error categories
    enum class ErrorCategory {
        General,
        Memory,
        Network,
        FileIO,
        DirectX,
        Game,
        Security,
        Threading,
        Configuration,
        Hardware
    };
    
    // Error recovery actions
    enum class RecoveryAction {
        None,
        Retry,
        Restart,
        Fallback,
        Ignore,
        Terminate
    };
    
    // Error information structure
    struct ErrorInfo {
        std::string errorCode;
        std::string description;
        std::string details;
        ErrorSeverity severity;
        ErrorCategory category;
        RecoveryAction recoveryAction;
        std::chrono::system_clock::time_point timestamp;
        std::string functionName;
        std::string fileName;
        int lineNumber;
        std::string stackTrace;
        std::map<std::string, std::string> context;
        bool isRecoverable;
        int retryCount;
        int maxRetries;
    };
    
    // Error handler interface
    class IErrorHandler {
    public:
        virtual ~IErrorHandler() = default;
        virtual bool HandleError(const ErrorInfo& error) = 0;
        virtual bool CanHandle(ErrorCategory category, ErrorSeverity severity) = 0;
        virtual std::string GetHandlerName() const = 0;
    };
    
    // Custom exception classes
    class RustToolkitException : public std::exception {
    private:
        std::string m_message;
        ErrorCategory m_category;
        ErrorSeverity m_severity;
        std::string m_functionName;
        std::string m_fileName;
        int m_lineNumber;
        
    public:
        RustToolkitException(const std::string& message, ErrorCategory category = ErrorCategory::General,
                           ErrorSeverity severity = ErrorSeverity::Error,
                           const std::string& functionName = "",
                           const std::string& fileName = "",
                           int lineNumber = 0)
            : m_message(message), m_category(category), m_severity(severity),
              m_functionName(functionName), m_fileName(fileName), m_lineNumber(lineNumber) {}
        
        const char* what() const noexcept override {
            return m_message.c_str();
        }
        
        ErrorCategory GetCategory() const { return m_category; }
        ErrorSeverity GetSeverity() const { return m_severity; }
        const std::string& GetFunctionName() const { return m_functionName; }
        const std::string& GetFileName() const { return m_fileName; }
        int GetLineNumber() const { return m_lineNumber; }
    };
    
    // Memory exception
    class MemoryException : public RustToolkitException {
    public:
        MemoryException(const std::string& message, const std::string& functionName = "",
                       const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Memory, ErrorSeverity::Critical,
                                 functionName, fileName, lineNumber) {}
    };
    
    // Network exception
    class NetworkException : public RustToolkitException {
    public:
        NetworkException(const std::string& message, const std::string& functionName = "",
                         const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Network, ErrorSeverity::Error,
                                 functionName, fileName, lineNumber) {}
    };
    
    // File I/O exception
    class FileIOException : public RustToolkitException {
    public:
        FileIOException(const std::string& message, const std::string& functionName = "",
                       const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::FileIO, ErrorSeverity::Error,
                                 functionName, fileName, lineNumber) {}
    };
    
    // DirectX exception
    class DirectXException : public RustToolkitException {
    private:
        HRESULT m_hresult;
        
    public:
        DirectXException(HRESULT hresult, const std::string& message,
                       const std::string& functionName = "",
                       const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::DirectX, ErrorSeverity::Critical,
                                 functionName, fileName, lineNumber), m_hresult(hresult) {}
        
        HRESULT GetHResult() const { return m_hresult; }
    };
    
    // Game exception
    class GameException : public RustToolkitException {
    public:
        GameException(const std::string& message, const std::string& functionName = "",
                      const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Game, ErrorSeverity::Error,
                                 functionName, fileName, lineNumber) {}
    };
    
    // Security exception
    class SecurityException : public RustToolkitException {
    public:
        SecurityException(const std::string& message, const std::string& functionName = "",
                          const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Security, ErrorSeverity::Critical,
                                 functionName, fileName, lineNumber) {}
    };
    
    // Threading exception
    class ThreadingException : public RustToolkitException {
    public:
        ThreadingException(const std::string& message, const std::string& functionName = "",
                          const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Threading, ErrorSeverity::Error,
                                 functionName, fileName, lineNumber) {}
    };
    
    // Configuration exception
    class ConfigurationException : public RustToolkitException {
    public:
        ConfigurationException(const std::string& message, const std::string& functionName = "",
                              const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Configuration, ErrorSeverity::Warning,
                                 functionName, fileName, lineNumber) {}
    };
    
    // Hardware exception
    class HardwareException : public RustToolkitException {
    public:
        HardwareException(const std::string& message, const std::string& functionName = "",
                         const std::string& fileName = "", int lineNumber = 0)
            : RustToolkitException(message, ErrorCategory::Hardware, ErrorSeverity::Critical,
                                 functionName, fileName, lineNumber) {}
    };
    
    // Error recovery manager
    class ErrorRecoveryManager {
    private:
        std::map<ErrorCategory, std::vector<std::shared_ptr<IErrorHandler>>> m_handlers;
        std::mutex m_mutex;
        
    public:
        void RegisterHandler(ErrorCategory category, std::shared_ptr<IErrorHandler> handler) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_handlers[category].push_back(handler);
        }
        
        bool HandleError(const ErrorInfo& error) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_handlers.find(error.category);
            if (it == m_handlers.end()) {
                return false;
            }
            
            for (auto& handler : it->second) {
                if (handler->CanHandle(error.category, error.severity)) {
                    if (handler->HandleError(error)) {
                        return true;
                    }
                }
            }
            
            return false;
        }
        
        std::vector<std::shared_ptr<IErrorHandler>> GetHandlers(ErrorCategory category) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_handlers.find(category);
            if (it != m_handlers.end()) {
                return it->second;
            }
            
            return {};
        }
    };
    
    // Stack trace utilities
    class StackTrace {
    public:
        static std::string GetCurrentStackTrace() {
            std::string stackTrace;
            
            HANDLE process = GetCurrentProcess();
            HANDLE thread = GetCurrentThread();
            
            CONTEXT context = {};
            context.ContextFlags = CONTEXT_FULL;
            
            if (RtlCaptureContext(&context)) {
                SymInitialize(process, nullptr, TRUE);
                
                STACKFRAME64 frame = {};
                frame.AddrPC.Offset = context.Rip;
                frame.AddrPC.Mode = AddrModeFlat;
                frame.AddrFrame.Offset = context.Rbp;
                frame.AddrFrame.Mode = AddrModeFlat;
                frame.AddrStack.Offset = context.Rsp;
                frame.AddrStack.Mode = AddrModeFlat;
                
                for (int i = 0; i < 10; i++) {
                    if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame,
                                   &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
                        break;
                    }
                    
                    char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
                    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
                    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                    symbol->MaxNameLen = MAX_SYM_NAME;
                    
                    DWORD64 displacement = 0;
                    if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
                        stackTrace += std::to_string(i) + ": " + std::string(symbol->Name, symbol->Name + symbol->NameLen) + "\n";
                    }
                }
                
                SymCleanup(process);
            }
            
            return stackTrace;
        }
        
        static std::string GetExceptionStackTrace(EXCEPTION_POINTERS* exceptionInfo) {
            std::string stackTrace;
            
            HANDLE process = GetCurrentProcess();
            HANDLE thread = GetCurrentThread();
            
            SymInitialize(process, nullptr, TRUE);
            
            STACKFRAME64 frame = {};
            frame.AddrPC.Offset = exceptionInfo->ContextRecord->Rip;
            frame.AddrPC.Mode = AddrModeFlat;
            frame.AddrFrame.Offset = exceptionInfo->ContextRecord->Rbp;
            frame.AddrFrame.Mode = AddrModeFlat;
            frame.AddrStack.Offset = exceptionInfo->ContextRecord->Rsp;
            frame.AddrStack.Mode = AddrModeFlat;
            
            for (int i = 0; i < 10; i++) {
                if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame,
                               exceptionInfo->ContextRecord, nullptr, SymFunctionTableAccess64,
                               SymGetModuleBase64, nullptr)) {
                    break;
                }
                
                char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
                SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                symbol->MaxNameLen = MAX_SYM_NAME;
                
                DWORD64 displacement = 0;
                if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
                    stackTrace += std::to_string(i) + ": " + std::string(symbol->Name, symbol->Name + symbol->NameLen) + "\n";
                }
            }
            
            SymCleanup(process);
            return stackTrace;
        }
    };
    
    // Comprehensive error handler
    class ComprehensiveErrorHandler {
    private:
        ErrorRecoveryManager m_recoveryManager;
        std::vector<ErrorInfo> m_errorHistory;
        std::map<std::string, int> m_errorCounts;
        std::mutex m_mutex;
        std::atomic<bool> m_criticalErrorOccurred;
        std::atomic<int> m_totalErrors;
        
        // Error handlers
        std::shared_ptr<IErrorHandler> m_memoryHandler;
        std::shared_ptr<IErrorHandler> m_networkHandler;
        std::shared_ptr<IErrorHandler> m_fileHandler;
        std::shared_ptr<IErrorHandler> m_directxHandler;
        std::shared_ptr<IErrorHandler> m_securityHandler;
        
    public:
        ComprehensiveErrorHandler() : m_criticalErrorOccurred(false), m_totalErrors(0) {
            InitializeHandlers();
        }
        
        void InitializeHandlers() {
            // Register default handlers
            m_memoryHandler = std::make_shared<MemoryErrorHandler>();
            m_networkHandler = std::make_shared<NetworkErrorHandler>();
            m_fileHandler = std::make_shared<FileIOErrorHandler>();
            m_directxHandler = std::make_shared<DirectXErrorHandler>();
            m_securityHandler = std::make_shared<SecurityErrorHandler>();
            
            m_recoveryManager.RegisterHandler(ErrorCategory::Memory, m_memoryHandler);
            m_recoveryManager.RegisterHandler(ErrorCategory::Network, m_networkHandler);
            m_recoveryManager.RegisterHandler(ErrorCategory::FileIO, m_fileHandler);
            m_recoveryManager.RegisterHandler(ErrorCategory::DirectX, m_directxHandler);
            m_recoveryManager.RegisterHandler(ErrorCategory::Security, m_securityHandler);
        }
        
        void HandleException(const std::exception& e, const std::string& functionName = "",
                            const std::string& fileName = "", int lineNumber = 0) {
            ErrorInfo error = CreateErrorFromException(e, functionName, fileName, lineNumber);
            ProcessError(error);
        }
        
        void HandleCustomException(const RustToolkitException& e) {
            ErrorInfo error = CreateErrorFromCustomException(e);
            ProcessError(error);
        }
        
        void HandleHRESULT(HRESULT hresult, const std::string& operation,
                           const std::string& functionName = "",
                           const std::string& fileName = "", int lineNumber = 0) {
            if (FAILED(hresult)) {
                DirectXException dxException(hresult, operation, functionName, fileName, lineNumber);
                ErrorInfo error = CreateErrorFromCustomException(dxException);
                ProcessError(error);
            }
        }
        
        void HandleError(ErrorCategory category, ErrorSeverity severity,
                         const std::string& message, const std::string& details = "",
                         RecoveryAction action = RecoveryAction::None,
                         const std::string& functionName = "",
                         const std::string& fileName = "", int lineNumber = 0) {
            ErrorInfo error = CreateError(category, severity, message, details, action,
                                        functionName, fileName, lineNumber);
            ProcessError(error);
        }
        
        bool HasCriticalError() const {
            return m_criticalErrorOccurred.load();
        }
        
        int GetTotalErrorCount() const {
            return m_totalErrors.load();
        }
        
        std::vector<ErrorInfo> GetErrorHistory() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_errorHistory;
        }
        
        std::map<std::string, int> GetErrorCounts() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_errorCounts;
        }
        
        void ClearErrorHistory() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_errorHistory.clear();
            m_errorCounts.clear();
            m_totalErrors.store(0);
            m_criticalErrorOccurred.store(false);
        }
        
    private:
        ErrorInfo CreateErrorFromException(const std::exception& e, const std::string& functionName,
                                       const std::string& fileName, int lineNumber) {
            ErrorInfo error;
            error.errorCode = "STD_EXCEPTION";
            error.description = e.what();
            error.severity = ErrorSeverity::Error;
            error.category = ErrorCategory::General;
            error.recoveryAction = RecoveryAction::None;
            error.timestamp = std::chrono::system_clock::now();
            error.functionName = functionName;
            error.fileName = fileName;
            error.lineNumber = lineNumber;
            error.stackTrace = StackTrace::GetCurrentStackTrace();
            error.isRecoverable = true;
            error.retryCount = 0;
            error.maxRetries = 3;
            
            return error;
        }
        
        ErrorInfo CreateErrorFromCustomException(const RustToolkitException& e) {
            ErrorInfo error;
            error.errorCode = "RUST_TOOLKIT_EXCEPTION";
            error.description = e.what();
            error.severity = e.GetSeverity();
            error.category = e.GetCategory();
            error.recoveryAction = RecoveryAction::None;
            error.timestamp = std::chrono::system_clock::now();
            error.functionName = e.GetFunctionName();
            error.fileName = e.GetFileName();
            error.lineNumber = e.GetLineNumber();
            error.stackTrace = StackTrace::GetCurrentStackTrace();
            error.isRecoverable = (e.GetSeverity() != ErrorSeverity::Fatal);
            error.retryCount = 0;
            error.maxRetries = 3;
            
            return error;
        }
        
        ErrorInfo CreateError(ErrorCategory category, ErrorSeverity severity,
                           const std::string& message, const std::string& details,
                           RecoveryAction action, const std::string& functionName,
                           const std::string& fileName, int lineNumber) {
            ErrorInfo error;
            error.errorCode = GenerateErrorCode(category, severity);
            error.description = message;
            error.details = details;
            error.severity = severity;
            error.category = category;
            error.recoveryAction = action;
            error.timestamp = std::chrono::system_clock::now();
            error.functionName = functionName;
            error.fileName = fileName;
            error.lineNumber = lineNumber;
            error.stackTrace = StackTrace::GetCurrentStackTrace();
            error.isRecoverable = (severity != ErrorSeverity::Fatal);
            error.retryCount = 0;
            error.maxRetries = 3;
            
            return error;
        }
        
        void ProcessError(ErrorInfo& error) {
            // Add to history
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_errorHistory.push_back(error);
                m_errorCounts[error.errorCode]++;
                m_totalErrors++;
                
                // Keep only last 1000 errors
                if (m_errorHistory.size() > 1000) {
                    m_errorHistory.erase(m_errorHistory.begin());
                }
            }
            
            // Log error
            LogError(error);
            
            // Check for critical error
            if (error.severity == ErrorSeverity::Critical || error.severity == ErrorSeverity::Fatal) {
                m_criticalErrorOccurred.store(true);
            }
            
            // Attempt recovery
            if (error.recoveryAction != RecoveryAction::None) {
                AttemptRecovery(error);
            }
            
            // Handle fatal errors
            if (error.severity == ErrorSeverity::Fatal) {
                HandleFatalError(error);
            }
        }
        
        void LogError(const ErrorInfo& error) {
            std::string severityString = SeverityToString(error.severity);
            std::string categoryString = CategoryToString(error.category);
            
            std::string logMessage = "[" + severityString + "][" + categoryString + "] " +
                                   error.errorCode + ": " + error.description;
            
            if (!error.details.empty()) {
                logMessage += " - " + error.details;
            }
            
            if (!error.functionName.empty()) {
                logMessage += " (Function: " + error.functionName;
                if (error.lineNumber > 0) {
                    logMessage += ", Line: " + std::to_string(error.lineNumber);
                }
                logMessage += ")";
            }
            
            LOG_ERROR(logMessage);
            
            if (!error.stackTrace.empty()) {
                LOG_ERROR("Stack Trace:\n" + error.stackTrace);
            }
        }
        
        void AttemptRecovery(ErrorInfo& error) {
            if (error.retryCount >= error.maxRetries) {
                return;
            }
            
            error.retryCount++;
            
            // Try recovery manager first
            if (m_recoveryManager.HandleError(error)) {
                LOG_INFO("Error recovery successful for: " + error.errorCode);
                return;
            }
            
            // Default recovery actions
            switch (error.recoveryAction) {
                case RecoveryAction::Retry:
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000 * error.retryCount));
                    break;
                    
                case RecoveryAction::Restart:
                    LOG_WARNING("Restart requested for error: " + error.errorCode);
                    // In a real implementation, this would restart the affected component
                    break;
                    
                case RecoveryAction::Fallback:
                    LOG_WARNING("Fallback activated for error: " + error.errorCode);
                    // In a real implementation, this would activate fallback behavior
                    break;
                    
                case RecoveryAction::Terminate:
                    LOG_ERROR("Termination requested for error: " + error.errorCode);
                    std::terminate();
                    break;
                    
                default:
                    break;
            }
        }
        
        void HandleFatalError(const ErrorInfo& error) {
            LOG_ERROR("FATAL ERROR: " + error.description);
            LOG_ERROR("System will terminate due to fatal error");
            
            // In a real implementation, this might:
            // 1. Save crash dump
            // 2. Send error report
            // 3. Clean up resources
            // 4. Terminate gracefully
            
            std::terminate();
        }
        
        std::string GenerateErrorCode(ErrorCategory category, ErrorSeverity severity) {
            std::string prefix = "RTK_";
            
            switch (category) {
                case ErrorCategory::Memory: prefix += "MEM_"; break;
                case ErrorCategory::Network: prefix += "NET_"; break;
                case ErrorCategory::FileIO: prefix += "IO_"; break;
                case ErrorCategory::DirectX: prefix += "DX_"; break;
                case ErrorCategory::Game: prefix += "GAME_"; break;
                case ErrorCategory::Security: prefix += "SEC_"; break;
                case ErrorCategory::Threading: prefix += "THR_"; break;
                case ErrorCategory::Configuration: prefix += "CFG_"; break;
                case ErrorCategory::Hardware: prefix += "HW_"; break;
                default: prefix += "GEN_"; break;
            }
            
            switch (severity) {
                case ErrorSeverity::Info: prefix += "INFO"; break;
                case ErrorSeverity::Warning: prefix += "WARN"; break;
                case ErrorSeverity::Error: prefix += "ERR"; break;
                case ErrorSeverity::Critical: prefix += "CRIT"; break;
                case ErrorSeverity::Fatal: prefix += "FATAL"; break;
                default: prefix += "UNK"; break;
            }
            
            return prefix + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 10000);
        }
        
        std::string SeverityToString(ErrorSeverity severity) const {
            switch (severity) {
                case ErrorSeverity::Info: return "INFO";
                case ErrorSeverity::Warning: return "WARNING";
                case ErrorSeverity::Error: return "ERROR";
                case ErrorSeverity::Critical: return "CRITICAL";
                case ErrorSeverity::Fatal: return "FATAL";
                default: return "UNKNOWN";
            }
        }
        
        std::string CategoryToString(ErrorCategory category) const {
            switch (category) {
                case ErrorCategory::Memory: return "MEMORY";
                case ErrorCategory::Network: return "NETWORK";
                case ErrorCategory::FileIO: return "FILE_IO";
                case ErrorCategory::DirectX: return "DIRECTX";
                case ErrorCategory::Game: return "GAME";
                case ErrorCategory::Security: return "SECURITY";
                case ErrorCategory::Threading: return "THREADING";
                case ErrorCategory::Configuration: return "CONFIGURATION";
                case ErrorCategory::Hardware: return "HARDWARE";
                default: return "GENERAL";
            }
        }
    };
    
    // Error handler implementations
    class MemoryErrorHandler : public IErrorHandler {
    public:
        bool HandleError(const ErrorInfo& error) override {
            if (error.severity == ErrorSeverity::Critical) {
                // Try to free memory
                LOG_WARNING("Attempting memory recovery");
                
                // In a real implementation, this would:
                // 1. Clear caches
                // 2. Free unused objects
                // 3. Compact memory
                // 4. Restart memory manager
                
                return true;
            }
            return false;
        }
        
        bool CanHandle(ErrorCategory category, ErrorSeverity severity) override {
            return category == ErrorCategory::Memory;
        }
        
        std::string GetHandlerName() const override {
            return "MemoryErrorHandler";
        }
    };
    
    class NetworkErrorHandler : public IErrorHandler {
    public:
        bool HandleError(const ErrorInfo& error) override {
            if (error.severity == ErrorSeverity::Error) {
                // Try to reconnect
                LOG_WARNING("Attempting network recovery");
                
                // In a real implementation, this would:
                // 1. Reset network connection
                // 2. Reconnect to server
                // 3. Retry failed requests
                
                return true;
            }
            return false;
        }
        
        bool CanHandle(ErrorCategory category, ErrorSeverity severity) override {
            return category == ErrorCategory::Network;
        }
        
        std::string GetHandlerName() const override {
            return "NetworkErrorHandler";
        }
    };
    
    class FileIOErrorHandler : public IErrorHandler {
    public:
        bool HandleError(const ErrorInfo& error) override {
            if (error.severity == ErrorSeverity::Error) {
                // Try to fix file access issues
                LOG_WARNING("Attempting file I/O recovery");
                
                // In a real implementation, this would:
                // 1. Check file permissions
                // 2. Create missing directories
                // 3. Retry file operations
                
                return true;
            }
            return false;
        }
        
        bool CanHandle(ErrorCategory category, ErrorSeverity severity) override {
            return category == ErrorCategory::FileIO;
        }
        
        std::string GetHandlerName() const override {
            return "FileIOErrorHandler";
        }
    };
    
    class DirectXErrorHandler : public IErrorHandler {
    public:
        bool HandleError(const ErrorInfo& error) override {
            if (error.severity == ErrorSeverity::Critical) {
                // Try to reset DirectX
                LOG_WARNING("Attempting DirectX recovery");
                
                // In a real implementation, this would:
                // 1. Reset DirectX device
                // 2. Recreate resources
                // 3. Reinitialize renderer
                
                return true;
            }
            return false;
        }
        
        bool CanHandle(ErrorCategory category, ErrorSeverity severity) override {
            return category == ErrorCategory::DirectX;
        }
        
        std::string GetHandlerName() const override {
            return "DirectXErrorHandler";
        }
    };
    
    class SecurityErrorHandler : public IErrorHandler {
    public:
        bool HandleError(const ErrorInfo& error) override {
            if (error.severity == ErrorSeverity::Critical) {
                // Try to restore security
                LOG_WARNING("Attempting security recovery");
                
                // In a real implementation, this would:
                // 1. Reinitialize encryption
                // 2. Restore security settings
                // 3. Restart protection systems
                
                return true;
            }
            return false;
        }
        
        bool CanHandle(ErrorCategory category, ErrorSeverity severity) override {
            return category == ErrorCategory::Security;
        }
        
        std::string GetHandlerName() const override {
            return "SecurityErrorHandler";
        }
    };
    
    // Global error handler instance
    inline std::unique_ptr<ComprehensiveErrorHandler> g_errorHandler = std::make_unique<ComprehensiveErrorHandler>();
    
    // Convenience macros
    #define RUST_TOOLKIT_TRY(operation) \
        try { \
            operation; \
        } catch (const std::exception& e) { \
            g_errorHandler->HandleException(e, __FUNCTION__, __FILE__, __LINE__); \
        } catch (...) { \
            g_errorHandler->HandleError(ErrorCategory::General, ErrorSeverity::Error, \
                                       "Unknown exception occurred", "", RecoveryAction::None, \
                                       __FUNCTION__, __FILE__, __LINE__); \
        }
    
    #define RUST_TOOLKIT_HRESULT_CHECK(hresult, operation) \
        if (FAILED(hresult)) { \
            g_errorHandler->HandleHRESULT(hresult, operation, __FUNCTION__, __FILE__, __LINE__); \
        }
    
    #define RUST_TOOLKIT_THROW(category, severity, message) \
        throw RustToolkitException(message, category, severity, __FUNCTION__, __FILE__, __LINE__)
    
    #define RUST_TOOLKIT_HANDLE_ERROR(category, severity, message, action) \
        g_errorHandler->HandleError(category, severity, message, "", action, __FUNCTION__, __FILE__, __LINE__)
    
} // namespace ErrorHandling
