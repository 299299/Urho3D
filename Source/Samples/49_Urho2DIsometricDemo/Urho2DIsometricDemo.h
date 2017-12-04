//
// Copyright (c) 2008-2014 the Urho3D project.
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

#pragma once

#include "Sample.h"
#include "Utilities2D/Sample2D.h"

class Character2D;
class Sample2D;

/// Urho2D tile map example.
/// This sample demonstrates:
///    - Creating an isometric 2D scene with tile map
///    - Displaying the scene using the Renderer subsystem
///    - Handling keyboard to move a character and zoom 2D camera
///    - Generating physics shapes from the tmx file's objects
///    - Displaying debug geometry for physics and tile map
/// Note that this sample uses some functions from Sample2D utility class.
class Urho2DIsometricDemo : public Sample
{
    URHO3D_OBJECT(Urho2DIsometricDemo, Sample);

public:
    /// Construct.
    Urho2DIsometricDemo(Context* context);

    /// Setup after engine initialization and before running the main loop.
    virtual void Start();
    /// Setup before engine initialization. Modifies the engine parameters.
    virtual void Setup();
    
private:
    /// Construct the scene content.
    void CreateScene();
    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Subscribe to application-wide logic update events.
    void SubscribeToEvents();
    /// Handle the logic update event.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle the logic post update event.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle the post render update event.
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle the end rendering event.
    void HandleSceneRendered(StringHash eventType, VariantMap& eventData);
    /// Handle reloading the scene.
    void ReloadScene(bool reInit);
    /// Handle the contact begin event (Box2D contact listener).
    void HandleCollisionBegin(StringHash eventType, VariantMap& eventData);
    /// Handle 'PLAY' button released event.
    void HandlePlayButton(StringHash eventType, VariantMap& eventData);

    /// The controllable character component.
    WeakPtr<Character2D> character2D_;
    /// Camera's zoom (used to scale movement speed based on camera zoom).
    float zoom_;
    /// Flag for drawing debug geometry.
    bool drawDebug_;

    /// Sample2D utility object.
    SharedPtr<Sample2D> sample2D_;
};
