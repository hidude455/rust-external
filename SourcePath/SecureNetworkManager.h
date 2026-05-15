/*
 * Secure Network Manager for Rust Anti-Cheat Evasion System
 * Provides SSL-verified network communications with certificate pinning
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/x509.h>
#include <Windows.h>
#include <wincrypt.h>

namespace Network {
    
    // SSL certificate information
    struct CertificateInfo {
        std::string subject;
        std::string issuer;
        std::string serialNumber;
        std::string fingerprint;
        std::vector<std::string> subjectAltNames;
        std::time_t validFrom;
        std::time_t validTo;
        bool isValid;
    };
    
    // SSL verification result
    struct SSLVerificationResult {
        bool success;
        std::string errorMessage;
        CertificateInfo certificateInfo;
        std::vector<std::string> warnings;
        bool certificatePinningValid;
        bool hostnameValid;
        bool certificateChainValid;
    };
    
    // Certificate pinning configuration
    struct CertificatePinningConfig {
        std::vector<std::string> pinnedCertificates;
        std::vector<std::string> pinnedPublicKeys;
        std::vector<std::string> pinnedFingerprints;
        bool enablePublicKeyPinning;
        bool enableCertificatePinning;
        bool enableFingerprintPinning;
        bool strictMode;
    };
    
    // SSL context manager
    class SSLContextManager {
    private:
        SSL_CTX* m_sslContext;
        std::string m_caBundlePath;
        std::vector<std::string> m_trustedCertificates;
        std::mutex m_mutex;
        bool m_initialized;
        
    public:
        SSLContextManager() : m_sslContext(nullptr), m_initialized(false) {
            Initialize();
        }
        
        ~SSLContextManager() {
            Cleanup();
        }
        
        bool Initialize() {
            if (m_initialized) return true;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Initialize OpenSSL
            SSL_library_init();
            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();
            
            // Create SSL context
            m_sslContext = SSL_CTX_new(TLS_client_method());
            if (!m_sslContext) {
                LOG_ERROR("Failed to create SSL context");
                return false;
            }
            
            // Set default verification options
            SSL_CTX_set_verify(m_sslContext, SSL_VERIFY_PEER, nullptr);
            SSL_CTX_set_verify_depth(m_sslContext, 4);
            
            // Load default CA certificates
            LoadDefaultCACertificates();
            
            m_initialized = true;
            return true;
        }
        
        void Cleanup() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (m_sslContext) {
                SSL_CTX_free(m_sslContext);
                m_sslContext = nullptr;
            }
            
            EVP_cleanup();
            ERR_free_strings();
            m_initialized = false;
        }
        
        bool LoadCACertificate(const std::string& certPath) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (!m_sslContext) return false;
            
            // Load CA certificate file
            if (SSL_CTX_load_verify_locations(m_sslContext, certPath.c_str(), nullptr) != 1) {
                LOG_ERROR("Failed to load CA certificate: " + certPath);
                return false;
            }
            
            m_trustedCertificates.push_back(certPath);
            return true;
        }
        
        bool LoadCACertificateFromMemory(const std::vector<uint8_t>& certData) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (!m_sslContext) return false;
            
            // Create BIO from memory
            BIO* bio = BIO_new_mem_buf(certData.data(), static_cast<int>(certData.size()));
            if (!bio) {
                LOG_ERROR("Failed to create BIO from memory");
                return false;
            }
            
            // Load certificate
            X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
            BIO_free(bio);
            
            if (!cert) {
                LOG_ERROR("Failed to parse certificate from memory");
                return false;
            }
            
            // Add to store
            X509_STORE* store = SSL_CTX_get_cert_store(m_sslContext);
            if (X509_STORE_add_cert(store, cert) != 1) {
                X509_free(cert);
                LOG_ERROR("Failed to add certificate to store");
                return false;
            }
            
            X509_free(cert);
            return true;
        }
        
        SSL_CTX* GetSSLContext() const {
            return m_sslContext;
        }
        
        bool IsInitialized() const {
            return m_initialized;
        }
        
    private:
        void LoadDefaultCACertificates() {
            // Try to load system CA certificates
            std::vector<std::string> caPaths = {
                "C:\\Program Files\\Common Files\\SSL\\cert.pem",
                "C:\\Program Files (x86)\\Common Files\\SSL\\cert.pem",
                "C:\\Windows\\System32\\curl-ca-bundle.crt",
                "ca-bundle.crt",
                "cacert.pem"
            };
            
            for (const auto& path : caPaths) {
                if (LoadCACertificate(path)) {
                    m_caBundlePath = path;
                    break;
                }
            }
        }
    };
    
    // Certificate verification utilities
    class CertificateVerifier {
    private:
        CertificatePinningConfig m_pinningConfig;
        std::mutex m_mutex;
        
    public:
        CertificateVerifier(const CertificatePinningConfig& config) : m_pinningConfig(config) {}
        
        SSLVerificationResult VerifyCertificate(const std::string& hostname, X509* cert) {
            SSLVerificationResult result;
            result.success = false;
            result.certificatePinningValid = true;
            result.hostnameValid = true;
            result.certificateChainValid = true;
            
            try {
                // Extract certificate information
                result.certificateInfo = ExtractCertificateInfo(cert);
                
                // Verify hostname
                result.hostnameValid = VerifyHostname(hostname, cert);
                if (!result.hostnameValid) {
                    result.errorMessage = "Hostname verification failed";
                    return result;
                }
                
                // Verify certificate validity period
                if (!result.certificateInfo.isValid) {
                    result.errorMessage = "Certificate is not valid or expired";
                    return result;
                }
                
                // Verify certificate pinning
                if (m_pinningConfig.enableCertificatePinning || 
                    m_pinningConfig.enablePublicKeyPinning || 
                    m_pinningConfig.enableFingerprintPinning) {
                    
                    result.certificatePinningValid = VerifyCertificatePinning(cert);
                    if (!result.certificatePinningValid) {
                        result.errorMessage = "Certificate pinning verification failed";
                        if (m_pinningConfig.strictMode) {
                            return result;
                        } else {
                            result.warnings.push_back("Certificate pinning failed but continuing in non-strict mode");
                        }
                    }
                }
                
                result.success = true;
            }
            catch (const std::exception& e) {
                result.errorMessage = "Certificate verification error: " + std::string(e.what());
            }
            
            return result;
        }
        
        void SetPinningConfig(const CertificatePinningConfig& config) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_pinningConfig = config;
        }
        
        CertificatePinningConfig GetPinningConfig() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_pinningConfig;
        }
        
    private:
        CertificateInfo ExtractCertificateInfo(X509* cert) {
            CertificateInfo info;
            
            if (!cert) return info;
            
            // Extract subject
            char subject[256];
            X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject));
            info.subject = subject;
            
            // Extract issuer
            char issuer[256];
            X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer));
            info.issuer = issuer;
            
            // Extract serial number
            ASN1_INTEGER* serial = X509_get_serialNumber(cert);
            if (serial) {
                BIGNUM* bn = ASN1_INTEGER_to_BN(serial, nullptr);
                if (bn) {
                    char* serialStr = BN_bn2hex(bn);
                    if (serialStr) {
                        info.serialNumber = serialStr;
                        OPENSSL_free(serialStr);
                    }
                    BN_free(bn);
                }
            }
            
            // Calculate fingerprint
            unsigned char fingerprint[SHA256_DIGEST_LENGTH];
            unsigned int fingerprintLen;
            
            if (X509_digest(cert, EVP_sha256(), fingerprint, &fingerprintLen)) {
                std::stringstream ss;
                for (unsigned int i = 0; i < fingerprintLen; i++) {
                    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(fingerprint[i]);
                }
                info.fingerprint = ss.str();
            }
            
            // Extract validity period
            ASN1_TIME* notBefore = X509_get_notBefore(cert);
            ASN1_TIME* notAfter = X509_get_notAfter(cert);
            
            info.validFrom = ASN1_TIME_to_time_t(notBefore);
            info.validTo = ASN1_TIME_to_time_t(notAfter);
            
            // Check if certificate is currently valid
            std::time_t currentTime = std::time(nullptr);
            info.isValid = (currentTime >= info.validFrom && currentTime <= info.validTo);
            
            // Extract Subject Alternative Names
            STACK_OF(GENERAL_NAME)* sanNames = static_cast<STACK_OF(GENERAL_NAME)*>(X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));
            if (sanNames) {
                for (int i = 0; i < sk_GENERAL_NAME_num(sanNames); i++) {
                    GENERAL_NAME* name = sk_GENERAL_NAME_value(sanNames, i);
                    if (name->type == GEN_DNS) {
                        char* dnsName = ASN1_STRING_get_data(name->d.dNSName);
                        info.subjectAltNames.push_back(std::string(dnsName));
                    }
                }
                sk_GENERAL_NAME_free(sanNames);
            }
            
            return info;
        }
        
        bool VerifyHostname(const std::string& hostname, X509* cert) {
            if (!cert) return false;
            
            // Check Common Name
            char subject[256];
            X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject));
            
            std::string subjectStr(subject);
            size_t cnPos = subjectStr.find("CN=");
            if (cnPos != std::string::npos) {
                cnPos += 3;
                size_t cnEnd = subjectStr.find(",", cnPos);
                std::string commonName = subjectStr.substr(cnPos, cnEnd - cnPos);
                
                if (commonName == hostname) {
                    return true;
                }
            }
            
            // Check Subject Alternative Names
            STACK_OF(GENERAL_NAME)* sanNames = static_cast<STACK_OF(GENERAL_NAME)*>(X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));
            if (sanNames) {
                for (int i = 0; i < sk_GENERAL_NAME_num(sanNames); i++) {
                    GENERAL_NAME* name = sk_GENERAL_NAME_value(sanNames, i);
                    if (name->type == GEN_DNS) {
                        char* dnsName = ASN1_STRING_get_data(name->d.dNSName);
                        if (std::string(dnsName) == hostname) {
                            sk_GENERAL_NAME_free(sanNames);
                            return true;
                        }
                    }
                }
                sk_GENERAL_NAME_free(sanNames);
            }
            
            return false;
        }
        
        bool VerifyCertificatePinning(X509* cert) {
            if (!cert) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Certificate pinning
            if (m_pinningConfig.enableCertificatePinning) {
                if (!VerifyCertificatePinningImpl(cert)) {
                    return false;
                }
            }
            
            // Public key pinning
            if (m_pinningConfig.enablePublicKeyPinning) {
                if (!VerifyPublicKeyPinningImpl(cert)) {
                    return false;
                }
            }
            
            // Fingerprint pinning
            if (m_pinningConfig.enableFingerprintPinning) {
                if (!VerifyFingerprintPinningImpl(cert)) {
                    return false;
                }
            }
            
            return true;
        }
        
        bool VerifyCertificatePinningImpl(X509* cert) {
            // Get certificate DER
            unsigned char* certDer = nullptr;
            int certDerLen = i2d_X509(cert, &certDer);
            
            if (certDerLen <= 0 || !certDer) {
                return false;
            }
            
            // Calculate SHA256 hash
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256(certDer, certDerLen, hash);
            
            // Convert to hex string
            std::stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            }
            std::string certHash = ss.str();
            
            // Check against pinned certificates
            for (const auto& pinned : m_pinningConfig.pinnedCertificates) {
                if (certHash == pinned) {
                    OPENSSL_free(certDer);
                    return true;
                }
            }
            
            OPENSSL_free(certDer);
            return false;
        }
        
        bool VerifyPublicKeyPinningImpl(X509* cert) {
            EVP_PKEY* pubKey = X509_get_pubkey(cert);
            if (!pubKey) return false;
            
            // Get public key bytes
            unsigned char* pubKeyDer = nullptr;
            int pubKeyDerLen = i2d_PUBKEY(pubKey, &pubKeyDer);
            
            if (pubKeyDerLen <= 0 || !pubKeyDer) {
                EVP_PKEY_free(pubKey);
                return false;
            }
            
            // Calculate SHA256 hash
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256(pubKeyDer, pubKeyDerLen, hash);
            
            // Convert to hex string
            std::stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            }
            std::string pubKeyHash = ss.str();
            
            // Check against pinned public keys
            for (const auto& pinned : m_pinningConfig.pinnedPublicKeys) {
                if (pubKeyHash == pinned) {
                    OPENSSL_free(pubKeyDer);
                    EVP_PKEY_free(pubKey);
                    return true;
                }
            }
            
            OPENSSL_free(pubKeyDer);
            EVP_PKEY_free(pubKey);
            return false;
        }
        
        bool VerifyFingerprintPinningImpl(X509* cert) {
            // Get certificate fingerprint
            unsigned char fingerprint[SHA256_DIGEST_LENGTH];
            unsigned int fingerprintLen;
            
            if (!X509_digest(cert, EVP_sha256(), fingerprint, &fingerprintLen)) {
                return false;
            }
            
            // Convert to hex string
            std::stringstream ss;
            for (unsigned int i = 0; i < fingerprintLen; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(fingerprint[i]);
            }
            std::string certFingerprint = ss.str();
            
            // Check against pinned fingerprints
            for (const auto& pinned : m_pinningConfig.pinnedFingerprints) {
                if (certFingerprint == pinned) {
                    return true;
                }
            }
            
            return false;
        }
    };
    
    // Secure network manager with SSL verification
    class SecureNetworkManager {
    private:
        std::unique_ptr<SSLContextManager> m_sslContext;
        std::unique_ptr<CertificateVerifier> m_certVerifier;
        std::string m_userAgent;
        std::mutex m_mutex;
        bool m_initialized;
        
        // CURL callback for SSL verification
        static curl_socket_t CurlOpenSocketCallback(curl_socket_t purpose,
                                                     struct curlsockaddr* address,
                                                     int socktype,
                                                     curlsocktype curlsocktype) {
            return socket(address->sa_family, socktype, 0);
        }
        
        static int CurlSSLCtxCallback(CURL* curl, void* sslctx, void* userptr) {
            SSL_CTX* ctx = static_cast<SSL_CTX*>(sslctx);
            
            // Set SSL options
            SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
            SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
            
            return CURLE_OK;
        }
        
        static int CurlSSLVerifyCallback(CURL* curl, void* sslctx, void* userptr) {
            SSL* ssl = static_cast<SSL*>(userptr);
            X509* cert = SSL_get_peer_certificate(ssl);
            
            if (!cert) {
                return 0; // No certificate
            }
            
            // Get hostname from CURL
            char* hostname = nullptr;
            curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &hostname);
            
            if (!hostname) {
                X509_free(cert);
                return 0;
            }
            
            // Verify certificate
            SecureNetworkManager* manager = static_cast<SecureNetworkManager*>(userptr);
            SSLVerificationResult result = manager->m_certVerifier->VerifyCertificate(hostname, cert);
            
            X509_free(cert);
            
            return result.success ? 1 : 0;
        }
        
    public:
        SecureNetworkManager() : m_initialized(false) {
            Initialize();
        }
        
        ~SecureNetworkManager() {
            Cleanup();
        }
        
        void Initialize() {
            if (m_initialized) return;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Initialize SSL context
            m_sslContext = std::make_unique<SSLContextManager>();
            
            // Initialize certificate verifier with default config
            CertificatePinningConfig config;
            config.enableCertificatePinning = true;
            config.enablePublicKeyPinning = true;
            config.enableFingerprintPinning = false;
            config.strictMode = false;
            
            m_certVerifier = std::make_unique<CertificateVerifier>(config);
            
            // Set user agent
            m_userAgent = "RustToolkit/1.0 (Secure Network Manager)";
            
            m_initialized = true;
        }
        
        void Cleanup() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            m_certVerifier.reset();
            m_sslContext.reset();
            m_initialized = false;
        }
        
        bool DownloadFile(const std::string& url, const std::string& destinationPath, 
                          SSLVerificationResult& verificationResult) {
            if (!m_initialized) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            CURL* curl = curl_easy_init();
            if (!curl) {
                verificationResult.errorMessage = "Failed to initialize CURL";
                return false;
            }
            
            // Set SSL options
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
            curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, CurlSSLCtxCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, m_sslContext->GetSSLContext());
            
            // Set URL and user agent
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, m_userAgent.c_str());
            
            // Set timeout
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            
            // Set output file
            FILE* file = nullptr;
            fopen_s(&file, destinationPath.c_str(), "wb");
            if (!file) {
                verificationResult.errorMessage = "Failed to create output file";
                curl_easy_cleanup(curl);
                return false;
            }
            
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            
            // Perform request
            CURLcode res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                // Get SSL verification result
                long sslVerifyResult;
                curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &sslVerifyResult);
                
                verificationResult.success = (sslVerifyResult == 0);
                if (!verificationResult.success) {
                    verificationResult.errorMessage = "SSL verification failed";
                }
            } else {
                verificationResult.success = false;
                verificationResult.errorMessage = curl_easy_strerror(res);
            }
            
            fclose(file);
            curl_easy_cleanup(curl);
            
            return verificationResult.success;
        }
        
        bool UploadFile(const std::string& url, const std::string& filePath,
                       SSLVerificationResult& verificationResult) {
            if (!m_initialized) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            CURL* curl = curl_easy_init();
            if (!curl) {
                verificationResult.errorMessage = "Failed to initialize CURL";
                return false;
            }
            
            // Set SSL options
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
            curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, CurlSSLCtxCallback);
            curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, m_sslContext->GetSSLContext());
            
            // Set URL and user agent
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, m_userAgent.c_str());
            
            // Set timeout
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            
            // Set upload
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            
            // Open file
            FILE* file = nullptr;
            fopen_s(&file, filePath.c_str(), "rb");
            if (!file) {
                verificationResult.errorMessage = "Failed to open file for upload";
                curl_easy_cleanup(curl);
                return false;
            }
            
            // Get file size
            fseek(file, 0, SEEK_END);
            curl_off_t fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            curl_easy_setopt(curl, CURLOPT_READDATA, file);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fileSize);
            
            // Perform request
            CURLcode res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                // Get SSL verification result
                long sslVerifyResult;
                curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &sslVerifyResult);
                
                verificationResult.success = (sslVerifyResult == 0);
                if (!verificationResult.success) {
                    verificationResult.errorMessage = "SSL verification failed";
                }
            } else {
                verificationResult.success = false;
                verificationResult.errorMessage = curl_easy_strerror(res);
            }
            
            fclose(file);
            curl_easy_cleanup(curl);
            
            return verificationResult.success;
        }
        
        bool AddTrustedCertificate(const std::string& certPath) {
            if (!m_initialized) return false;
            
            return m_sslContext->LoadCACertificate(certPath);
        }
        
        bool AddTrustedCertificateFromMemory(const std::vector<uint8_t>& certData) {
            if (!m_initialized) return false;
            
            return m_sslContext->LoadCACertificateFromMemory(certData);
        }
        
        void SetCertificatePinningConfig(const CertificatePinningConfig& config) {
            if (m_certVerifier) {
                m_certVerifier->SetPinningConfig(config);
            }
        }
        
        CertificatePinningConfig GetCertificatePinningConfig() const {
            return m_certVerifier->GetPinningConfig();
        }
        
        void SetUserAgent(const std::string& userAgent) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_userAgent = userAgent;
        }
        
        std::string GetUserAgent() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_userAgent;
        }
        
        bool IsInitialized() const {
            return m_initialized;
        }
        
    };
    
} // namespace Network
