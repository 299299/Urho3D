//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>

#ifdef URHO3D_LUA
#include <Urho3D/LuaScript/LuaScript.h>
#endif

#ifdef URHO3D_ANGELSCRIPT
#include <Urho3D/AngelScript/APITemplates.h>
#endif
#include "TailGenerator.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <Urho3D/DebugNew.h>

using namespace Urho3D;

static void RegisterCustomLibrary(Context* context)
{
#ifdef URHO3D_ANGELSCRIPT
    Script* script = context->GetSubsystem<Script>();
    asIScriptEngine* engine = script->GetScriptEngine();
    RegisterDrawable<TailGenerator>(engine, "TailGenerator");
    engine->RegisterObjectMethod("TailGenerator", "void set_material(Material@+)", asMETHOD(TailGenerator, SetMaterial), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void set_endNodeName(const String&in)", asMETHOD(TailGenerator, SetEndNodeName), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void set_width(float)", asMETHOD(TailGenerator, SetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void set_tailNum(int)", asMETHOD(TailGenerator, SetTailNum), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void SetStartColor(const Color&in, const Color&in)", asMETHOD(TailGenerator, SetStartColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void SetEndColor(const Color&in, const Color&in)", asMETHOD(TailGenerator, SetEndColor), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void SetArcValue(float, float)", asMETHOD(TailGenerator, SetArcValue), asCALL_THISCALL);
    engine->RegisterObjectMethod("TailGenerator", "void set_relativePosition(bool)", asMETHOD(TailGenerator, SetRelativePosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("Camera", "Matrix4 GetProjection(bool) const", asMETHODPR(Camera, GetProjection, (bool) const, Matrix4), asCALL_THISCALL);
#endif
}


void CompileScript(Context* context, const String& fileName);

int main(int argc, char** argv)
{
    #ifdef WIN32
    const Vector<String>& arguments = ParseArguments(GetCommandLineW());
    #else
    const Vector<String>& arguments = ParseArguments(argc, argv);
    #endif

    bool dumpApiMode = false;
    String sourceTree;
    String outputFile;

    if (arguments.Size() < 1)
        ErrorExit("Usage: ScriptCompiler <input file> [resource path for includes]\n"
                  "       ScriptCompiler -dumpapi <source tree> <Doxygen output file> [C header output file]");
    else
    {
        if (arguments[0] != "-dumpapi")
            outputFile = arguments[0];
        else
        {
            dumpApiMode = true;
            if (arguments.Size() > 2)
            {
                sourceTree = arguments[1];
                outputFile = arguments[2];
            }
            else
                ErrorExit("Usage: ScriptCompiler -dumpapi <source tree> <Doxygen output file> [C header output file]");
        }
    }

    SharedPtr<Context> context(new Context());
    SharedPtr<Engine> engine(new Engine(context));
    context->RegisterSubsystem(new Script(context));

    // In API dumping mode initialize the engine and instantiate LuaScript system if available so that we
    // can dump attributes from as many classes as possible
    if (dumpApiMode)
    {
        VariantMap engineParameters;
        engineParameters["Headless"] = true;
        engineParameters["WorkerThreads"] = false;
        engineParameters["LogName"] = String::EMPTY;
        engineParameters["ResourcePaths"] = String::EMPTY;
        engineParameters["AutoloadPaths"] = String::EMPTY;
        engine->Initialize(engineParameters);
    #ifdef URHO3D_LUA
        context->RegisterSubsystem(new LuaScript(context));
    #endif
    }

    Log* log = context->GetSubsystem<Log>();
    // Register Log subsystem manually if compiled without logging support
    if (!log)
    {
        context->RegisterSubsystem(new Log(context));
        log = context->GetSubsystem<Log>();
    }

    log->SetLevel(LOG_WARNING);
    log->SetTimeStamp(false);

    RegisterCustomLibrary(context);

    if (!dumpApiMode)
    {
        String path, file, extension;
        SplitPath(outputFile, path, file, extension);

        ResourceCache* cache = context->GetSubsystem<ResourceCache>();

        // Add resource path to be able to resolve includes
        if (arguments.Size() > 1)
            cache->AddResourceDir(arguments[1]);
        else
            cache->AddResourceDir(cache->GetPreferredResourceDir(path));

        if (!file.StartsWith("*"))
            CompileScript(context, outputFile);
        else
        {
            Vector<String> scriptFiles;
            context->GetSubsystem<FileSystem>()->ScanDir(scriptFiles, path, file + extension, SCAN_FILES, false);
            for (unsigned i = 0; i < scriptFiles.Size(); ++i)
                CompileScript(context, path + scriptFiles[i]);
        }
    }
    else
    {
        if (!outputFile.Empty())
        {
            log->SetQuiet(true);
            log->Open(outputFile);
        }
        // If without output file, dump to stdout instead
        context->GetSubsystem<Script>()->DumpAPI(DOXYGEN, sourceTree);

        // Only dump API as C Header when an output file name is explicitly given
        if (arguments.Size() > 3)
        {
            outputFile = arguments[3];
            log->Open(outputFile);
            context->GetSubsystem<Script>()->DumpAPI(C_HEADER, sourceTree);
        }
    }

    return EXIT_SUCCESS;
}

void CompileScript(Context* context, const String& fileName)
{
    PrintLine("Compiling script file " + fileName);

    File inFile(context, fileName, FILE_READ);
    if (!inFile.IsOpen())
        ErrorExit("Failed to open script file " + fileName);

    ScriptFile script(context);
    if (!script.Load(inFile))
        ErrorExit();

    String outFileName = ReplaceExtension(fileName, ".asc");
    File outFile(context, outFileName, FILE_WRITE);
    if (!outFile.IsOpen())
        ErrorExit("Failed to open output file " + fileName);

    script.SaveByteCode(outFile);
}
