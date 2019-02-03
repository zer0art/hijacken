#include "Commands.h"
#include <iostream>

namespace Commands
{
    // =================

    ImpersonationOptions::ImpersonationOptions() : 
        _tokenSourceProcessId(::GetCurrentProcessId()), 
        _tokenIntegrityLevel(System::TokenIntegrityLvl::Untrusted),
        _changeIntegrityLevel(false)
    {
    }

    void ImpersonationOptions::LoadArgs(Utils::Arguments& args)
    {
        std::wstring command;

        if (!args.Probe(command))
            return;

        if (command == L"/impersonate")
        {
            args.SwitchToNext();

            if (!args.GetNext(command))
                throw Utils::Exception(L"Invalid impersonation argument '%s'", command.c_str());

            _tokenSourceProcessId = _wtoi(command.c_str());

            if (!args.Probe(command))
                return;
        }
        
        if (command == L"/integrity")
        {
            args.SwitchToNext();

            if (!args.GetNext(command))
                throw Utils::Exception(L"Invalid impersonation argument '%s'", command.c_str());

            _tokenIntegrityLevel = ConvertStrToIntegrityLevel(command);
            _changeIntegrityLevel = true;
        }
    }

    System::ImpersonateTokenPtr ImpersonationOptions::CraftToken()
    {
        System::Process target(_tokenSourceProcessId, PROCESS_QUERY_INFORMATION);
        auto token = System::ImpersonateTokenPtr(new System::ImpersonateToken(target, TOKEN_ADJUST_DEFAULT | TOKEN_QUERY /*| TOKEN_ASSIGN_PRIMARY*/));

        if (_changeIntegrityLevel)
            ChangeIntegrity(token, _tokenIntegrityLevel);

        return token;
    }

    void ImpersonationOptions::ChangeIntegrity(System::ImpersonateTokenPtr& token, System::TokenIntegrityLvl)
    {
        auto expectedIntegrity = _tokenIntegrityLevel;
        auto currentIntegrity = token->GetIntegrityLevel();

        if (currentIntegrity == expectedIntegrity)
            return;

        if (currentIntegrity < expectedIntegrity)
            throw Utils::Exception(L"Access token can't be elevated to higher integrity level");

        if (currentIntegrity == System::TokenIntegrityLvl::High)
        {
            token.reset(new System::ImpersonateToken(token->GetLinkedToken()));
            currentIntegrity = token->GetIntegrityLevel();
            if (currentIntegrity == System::TokenIntegrityLvl::High)
                throw Utils::Exception(L"Attempt to elevate token has been failed");
        }

        if (currentIntegrity != expectedIntegrity)
            token->SetIntegrityLevel(expectedIntegrity);

        if (token->GetIntegrityLevel() == expectedIntegrity)
            return;

        throw Utils::Exception(L"Attempt to prepare token with specific integrity level has been failed");
    }

    System::TokenIntegrityLvl ImpersonationOptions::ConvertStrToIntegrityLevel(std::wstring& level)
    {
        System::TokenIntegrityLvl integrity = System::TokenIntegrityLvl::Untrusted;

        if (level == L"medium")
            integrity = System::TokenIntegrityLvl::Medium;
        else if (level == L"low")
            integrity = System::TokenIntegrityLvl::Low;
        else if (level == L"untrusted")
            integrity = System::TokenIntegrityLvl::Untrusted;
        else
            throw Utils::Exception(L"Unknown integrity level '%d'", integrity);

        return integrity;
    }

// =================

    ScanFile::ScanFile()
    {
    }

    void ScanFile::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanFile::Perform()
    {
    }

// =================

    ScanDirectory::ScanDirectory()
    {
    }

    void ScanDirectory::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanDirectory::Perform()
    {
    }

// =================

    ScanProcess::ScanProcess() : _targetProcessId(0)
    {
    }

    void ScanProcess::LoadArgs(Utils::Arguments& args)
    {
        std::wstring command;

        if (!args.GetNext(command))
            throw Utils::Exception(L"Invalid command argument '%s'", command.c_str());

        _targetProcessId = _wtoi(command.c_str());

        ImpersonationOptions::LoadArgs(args);
    }

    void ScanProcess::Perform()
    {
        auto token = ImpersonationOptions::CraftToken();
        System::TokenAccessChecker access(*token);

        std::wcout << L"Scan process " << _targetProcessId << L", " << std::endl;
        std::wcout << L" Integrity level: " << token->GetIntegrityLevel() << std::endl;

        Engine::ProcessScanEngine::Scan(_targetProcessId, access);

        for (auto& detection : _detectedDirs)
        {
            
        }
    }

    void ScanProcess::NotifyWritableDirectory(DetectionDirType detection, std::wstring& dirPath)
    {
        _detectedDirs[detection].insert(dirPath);
    }

    void ScanProcess::NotifyWritableFile(DetectionFileType detection, std::wstring& filePath)
    {
        _detectedFiles[detection].insert(filePath);
    }

// =================

    ScanProcesses::ScanProcesses()
    {
    }

    void ScanProcesses::LoadArgs(Utils::Arguments& args)
    {
        ImpersonationOptions::LoadArgs(args);
    }

    void ScanProcesses::Perform()
    {
        auto token = ImpersonationOptions::CraftToken();
        System::TokenAccessChecker access(*token);

        DWORD processId;
        std::wstring processName;
        System::ProcessesSnapshot snapshot;

        while (snapshot.GetNextProcess(processId, processName))
        {
            enum { IdlePID = 0, SystemPID = 4 };

            if (processId == IdlePID || processId == SystemPID)
                continue;

            try
            {
                std::wcout << L"Scan process " << processId << L", " << processName.c_str() << std::endl;
                Engine::ProcessScanEngine::Scan(processId, access);
            }
            catch (Utils::Exception& exception)
            {
                std::wcout << L"Process with ID " << processId << L" has been skipped, reason: " << exception.GetMessage() << std::endl;
                continue;
            }
        }
    }

    void ScanProcesses::NotifyWritableDirectory(DetectionDirType detection, std::wstring& dirPath)
    {
        std::wcout << L" dir: " << (int)detection << L" " << dirPath.c_str() << std::endl;
    }

    void ScanProcesses::NotifyWritableFile(DetectionFileType detection, std::wstring& filePath)
    {
        std::wcout << L" file: " << (int)detection << L" " << filePath.c_str() << std::endl;
    }

// =================

    ScanAutorun::ScanAutorun()
    {
    }

    void ScanAutorun::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanAutorun::Perform()
    {
    }

// =================

    ScanTask::ScanTask()
    {
    }

    void ScanTask::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanTask::Perform()
    {
    }

// =================

    ScanTasks::ScanTasks()
    {
    }

    void ScanTasks::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanTasks::Perform()
    {
    }

// =================

    ScanService::ScanService()
    {
    }

    void ScanService::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanService::Perform()
    {
    }

// =================

    ScanServices::ScanServices()
    {
    }

    void ScanServices::LoadArgs(Utils::Arguments& args)
    {
    }

    void ScanServices::Perform()
    {
    }

// =================

    ScanSystem::ScanSystem()
    {
        System::Process self(::GetCurrentProcess());
        System::PrimaryToken token(self);
        token.SetPrivilege(L"SeDebugPrivilege", true);
    }

    void ScanSystem::LoadArgs(Utils::Arguments& args)
    {
        std::wstring command;

        _tokenSourceId = ::GetCurrentProcessId();

        if (!args.Probe(command))
            return;

        if (command != L"/toksrc")
            throw Utils::Exception(L"Invalid command '%s'", command.c_str());

        args.SwitchToNext();

        if (!args.GetNext(command))
            throw Utils::Exception(L"Invalid command argument '%s'", command.c_str());

        _tokenSourceId = _wtoi(command.c_str());
    }

    void ScanSystem::Perform()
    {
        System::Process process(_tokenSourceId, PROCESS_QUERY_INFORMATION);
        System::ImpersonateToken token(process);
        System::TokenAccessChecker access(token);

        DWORD processId;
        std::wstring processName;
        System::ProcessesSnapshot snapshot;

        while (snapshot.GetNextProcess(processId, processName))
        {
            enum { IdlePID = 0, SystemPID = 4 };

            if (processId == IdlePID || processId == SystemPID)
                continue;

            try
            {
                System::ProcessInformation info(processId);
                System::ProcessEnvironmentBlock peb(info);

                std::wcout << L"Scan process " << processId << L", " << processName.c_str() << std::endl;
                std::wcout << L" Integrity level: " << token.GetIntegrityLevel() << std::endl;

                ScanImage(access, info);
                ScanCurrentDirectory(access, peb);
                ScanEnvironmentPaths(access, peb);
                ScanModules(access, info);
            }
            catch (Utils::Exception& exception)
            {
                std::wcout << L"Process with ID " << processId << L" has been skipped, reason: " << exception.GetMessage() << std::endl;
                continue;
            }
        }
    }

    void ScanSystem::ScanImage(System::TokenAccessChecker& access, System::ProcessInformation& info)
    {
        try
        {
            std::wstring imageDir;
            info.GetImageDirectory(imageDir);

            if (IsDirWritable(imageDir, access))
            {
                std::wcout << L" [Img] " << imageDir.c_str() << std::endl;
                return;
            }

            std::wstring imageFile;
            info.GetImagePath(imageFile);

            if (IsFileWritable(imageFile, access))
            {
                std::wcout << L" [Img] " << imageFile.c_str() << std::endl;
                return;
            }
        }
        catch (...)
        {
            std::wcout << L" Skipped image scan" << std::endl;
        }
    }

    void ScanSystem::ScanCurrentDirectory(System::TokenAccessChecker& access, System::ProcessEnvironmentBlock& peb)
    {
        try
        {
            std::wstring currentDirPath;
            peb.GetCurrentDir(currentDirPath);

            if (IsDirWritable(currentDirPath, access))
            {
                std::wcout << L" [Cur] " << currentDirPath.c_str() << std::endl;
                return;
            }
        }
        catch (...)
        {
            std::wcout << L" Skipped current dir scan" << std::endl;
        }
    }

    void ScanSystem::ScanEnvironmentPaths(System::TokenAccessChecker& access, System::ProcessEnvironmentBlock& peb)
    {
        std::wstring pathSet;
        auto env = peb.GetProcessEnvironment();

        if (!env->GetValue(L"Path", pathSet) && !env->GetValue(L"PATH", pathSet) && !env->GetValue(L"path", pathSet))
            throw Utils::Exception(L"Can't obtain 'Path' environment variable");

        Utils::SeparatedStrings paths(pathSet, L';');

        for (auto& dir : paths)
        {
            try
            {
                if (IsDirWritable(dir, access))
                    std::wcout << L" [Env] " << dir.c_str() << std::endl;
            }
            catch (...)
            {
            }
        }
    }

    void ScanSystem::ScanModules(System::TokenAccessChecker& access, System::ProcessInformation& info)
    {
        System::ModulesSnapshot snapshot(info.GetProcess()->GetProcessID());
        HMODULE module;
        
        while (snapshot.GetNextModule(module))
        {
            std::wstring modulePath, moduleDir;
            info.GetModulePath(module, modulePath);

            Utils::ExtractFileDirectory(modulePath, moduleDir);
            
            if (IsDirWritable(moduleDir, access))
            {
                std::wcout << L" [Mod] " << moduleDir.c_str() << std::endl;
                continue;
            }

            if (IsFileWritable(modulePath, access))
            {
                std::wcout << L" [Mod] " << modulePath.c_str() << std::endl;
                continue;
            }
        }
    }

    bool ScanSystem::IsFileWritable(std::wstring path, System::TokenAccessChecker& access)
    {
        System::File file(path.c_str());
        System::SecurityDescriptor descriptor(file);
        return access.IsFileObjectAccessible(descriptor, FILE_WRITE_DATA);
    }

    bool ScanSystem::IsDirWritable(std::wstring path, System::TokenAccessChecker& access)
    {
        System::Directory directory(path.c_str());
        System::SecurityDescriptor descriptor(directory);
        return access.IsFileObjectAccessible(descriptor, FILE_ADD_FILE);
    }

// =================

    MakeDll::MakeDll()
    {
    }

    void MakeDll::LoadArgs(Utils::Arguments& args)
    {
    }

    void MakeDll::Perform()
    {
    }
};