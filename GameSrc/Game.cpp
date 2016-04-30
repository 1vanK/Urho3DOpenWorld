#include "Urho3DAll.h"

#define CACHE GetSubsystem<ResourceCache>()

#define CHUNK_SIZE 100.0f

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

public:
    SharedPtr<Node>
        chunkPosXPosZ_, chunkPosXCenterZ_, chunkPosXNegZ_,
        chunkCenterXPosZ_, chunkCenterXCenterZ_, chunkCenterXNegZ_,
        chunkNegXPosZ_, chunkNegXCenterZ_, chunkNegXNegZ_;

    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    float yaw_;
    float pitch_;

    Game(Context* context) :
        Application(context),
        yaw_(0.0f),
        pitch_(0.0f)
    {
    }

    SharedPtr<Node> GenerateChunk()
    //Node* GenerateChunk()
    {
        SharedPtr<Node> result(new Node(context_));
        //auto result = new Node(context_);

        Node* planeNode = result->CreateChild();
        planeNode->SetScale(Vector3(CHUNK_SIZE, 1.0f, CHUNK_SIZE));
        StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
        planeObject->SetModel(CACHE->GetResource<Model>("Models/Plane.mdl"));
        planeObject->SetMaterial(CACHE->GetResource<Material>("Materials/StoneTiled.xml"));

        Node* sphereNode = result->CreateChild();
        sphereNode->SetScale(Vector3(2.0f, 2.0f, 2.0f));
        StaticModel* sphereObject = sphereNode->CreateComponent<StaticModel>();
        sphereObject->SetModel(CACHE->GetResource<Model>("Models/Sphere.mdl"));
        //auto material = CACHE->GetResource<Material>("Materials/DefaultGrey.xml")->Clone();
        //material->SetShaderParameter("MatDiffColor", Color(Random(1.0f), Random(1.0f), Random(1.0f)));
        //sphereObject->SetMaterial(material);
        sphereObject->SetCastShadows(true);

        return result;
    }

    void Setup()
    {
        engineParameters_["WindowTitle"] = GetTypeName();
        //    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
        engineParameters_["FullScreen"] = false;
        engineParameters_["Headless"] = false;
        engineParameters_["WindowWidth"] = 800;
        engineParameters_["WindowHeight"] = 600;
        //    engineParameters_["ResourcePaths"] = "Data;CoreData;MyData";
    }

    void Start()
    {
        CreateScene();
        SetupViewport();
        SubscribeToEvents();

        ResourceCache* cache = GetSubsystem<ResourceCache>();
        XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
        DebugHud* debugHud = engine_->CreateDebugHud();
        debugHud->SetDefaultStyle(xmlFile);
    }

    void SetupViewport()
    {
        Renderer* renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);
    }

    void CreateScene()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        scene_ = new Scene(context_);
        scene_->CreateComponent<Octree>();

        Node* lightNode = scene_->CreateChild("DirectionalLight");
        lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
        Light* light = lightNode->CreateComponent<Light>();
        light->SetColor(Color(0.5f, 0.5f, 0.5f));
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
        light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
        //light->SetShadowIntensity(0.5f);

        Node* zoneNode = scene_->CreateChild("Zone");
        Zone* zone = zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
        zone->SetAmbientColor(Color(0.5f, 0.5f, 0.5f));
        zone->SetFogColor(Color(0.4f, 0.5f, 0.8f));
        zone->SetFogStart(1.0f);
        zone->SetFogEnd(100.0f);

        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->CreateComponent<Camera>();
        cameraNode_->SetPosition(Vector3(0.0f, 1.0f, 0.0f));

        chunkPosXPosZ_ = GenerateChunk();
        OffsetNode(chunkPosXPosZ_, Vector3(CHUNK_SIZE, 0.0f, CHUNK_SIZE));
        scene_->AddChild(chunkPosXPosZ_);

        chunkPosXCenterZ_ = GenerateChunk();
        OffsetNode(chunkPosXCenterZ_, Vector3(CHUNK_SIZE, 0.0f, 0.0f));
        scene_->AddChild(chunkPosXCenterZ_);

        chunkPosXNegZ_ = GenerateChunk();
        OffsetNode(chunkPosXNegZ_, Vector3(CHUNK_SIZE, 0.0f, -CHUNK_SIZE));
        scene_->AddChild(chunkPosXNegZ_);

        chunkCenterXPosZ_ = GenerateChunk();
        OffsetNode(chunkCenterXPosZ_, Vector3(0.0f, 0.0f, CHUNK_SIZE));
        scene_->AddChild(chunkCenterXPosZ_);

        chunkCenterXCenterZ_ = GenerateChunk();
        scene_->AddChild(chunkCenterXCenterZ_);

        chunkCenterXNegZ_ = GenerateChunk();
        OffsetNode(chunkCenterXNegZ_, Vector3(0.0f, 0.0f, -CHUNK_SIZE));
        scene_->AddChild(chunkCenterXNegZ_);

        chunkNegXPosZ_ = GenerateChunk();
        OffsetNode(chunkNegXPosZ_, Vector3(-CHUNK_SIZE, 0.0f, CHUNK_SIZE));
        scene_->AddChild(chunkNegXPosZ_);

        chunkNegXCenterZ_ = GenerateChunk();
        OffsetNode(chunkNegXCenterZ_, Vector3(-CHUNK_SIZE, 0.0f, 0.0f));
        scene_->AddChild(chunkNegXCenterZ_);

        chunkNegXNegZ_ = GenerateChunk();
        OffsetNode(chunkNegXNegZ_, Vector3(-CHUNK_SIZE, 0.0f, -CHUNK_SIZE));
        scene_->AddChild(chunkNegXNegZ_);
    }

    void OffsetNode(Node* chunk, Vector3 offset)
    {
        //chunk->SetPositionSilent(chunk->GetPosition() + offset); // doesn't works
        chunk->SetPosition(chunk->GetPosition() + offset);
        
    }

    void MoveCamera(float timeStep)
    {
        Input* input = GetSubsystem<Input>();

        const float MOVE_SPEED = 20.0f;
        const float MOUSE_SENSITIVITY = 0.1f;

        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

        if (input->GetKeyDown('W'))
            cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
        if (input->GetKeyDown('S'))
            cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
        if (input->GetKeyDown('A'))
            cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
        if (input->GetKeyDown('D'))
            cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

        if (input->GetKeyPress(KEY_F2))
            GetSubsystem<DebugHud>()->ToggleAll();
    }

    void SubscribeToEvents()
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;

        float timeStep = eventData[P_TIMESTEP].GetFloat();

        MoveCamera(timeStep);

        auto cameraPos = cameraNode_->GetPosition();
        if (cameraPos.x_ > CHUNK_SIZE / 2)
        {
            auto offset = Vector3(-CHUNK_SIZE, 0.0f, 0.0f);

            chunkNegXPosZ_->Remove();
            chunkNegXCenterZ_->Remove();
            chunkNegXNegZ_->Remove();

            chunkNegXPosZ_ = chunkCenterXPosZ_;
            chunkNegXCenterZ_ = chunkCenterXCenterZ_;
            chunkNegXNegZ_ = chunkCenterXNegZ_;

            OffsetNode(chunkNegXPosZ_, offset);
            OffsetNode(chunkNegXCenterZ_, offset);
            OffsetNode(chunkNegXNegZ_, offset);

            chunkCenterXPosZ_ = chunkPosXPosZ_;
            chunkCenterXCenterZ_ = chunkPosXCenterZ_;
            chunkCenterXNegZ_ = chunkPosXNegZ_;

            OffsetNode(chunkCenterXPosZ_, offset);
            OffsetNode(chunkCenterXCenterZ_, offset);
            OffsetNode(chunkCenterXNegZ_, offset);

            chunkPosXPosZ_ = GenerateChunk();
            chunkPosXCenterZ_ = GenerateChunk();
            chunkPosXNegZ_ = GenerateChunk();

            scene_->AddChild(chunkPosXPosZ_);
            scene_->AddChild(chunkPosXCenterZ_);
            scene_->AddChild(chunkPosXNegZ_);

            OffsetNode(chunkPosXPosZ_, -offset + Vector3(0.0f, 0.0f, CHUNK_SIZE));
            OffsetNode(chunkPosXCenterZ_, -offset);
            OffsetNode(chunkPosXNegZ_, -offset + Vector3(0.0f, 0.0f, -CHUNK_SIZE));

            OffsetNode(cameraNode_, offset);
        }

        if (cameraPos.x_ < -CHUNK_SIZE / 2)
        {
            auto offset = Vector3(CHUNK_SIZE, 0.0f, 0.0f);

            chunkPosXPosZ_->Remove();
            chunkPosXCenterZ_->Remove();
            chunkPosXNegZ_->Remove();

            chunkPosXPosZ_ = chunkCenterXPosZ_;
            chunkPosXCenterZ_ = chunkCenterXCenterZ_;
            chunkPosXNegZ_ = chunkCenterXNegZ_;

            OffsetNode(chunkPosXPosZ_, offset);
            OffsetNode(chunkPosXCenterZ_, offset);
            OffsetNode(chunkPosXNegZ_, offset);

            chunkCenterXPosZ_ = chunkNegXPosZ_;
            chunkCenterXCenterZ_ = chunkNegXCenterZ_;
            chunkCenterXNegZ_ = chunkNegXNegZ_;

            OffsetNode(chunkCenterXPosZ_, offset);
            OffsetNode(chunkCenterXCenterZ_, offset);
            OffsetNode(chunkCenterXNegZ_, offset);

            chunkNegXPosZ_ = GenerateChunk();
            chunkNegXCenterZ_ = GenerateChunk();
            chunkNegXNegZ_ = GenerateChunk();

            scene_->AddChild(chunkNegXPosZ_);
            scene_->AddChild(chunkNegXCenterZ_);
            scene_->AddChild(chunkNegXNegZ_);

            OffsetNode(chunkNegXPosZ_, -offset + Vector3(0.0f, 0.0f, CHUNK_SIZE));
            OffsetNode(chunkNegXCenterZ_, -offset);
            OffsetNode(chunkNegXNegZ_, -offset + Vector3(0.0f, 0.0f, -CHUNK_SIZE));

            OffsetNode(cameraNode_, offset);
        }

        if (cameraPos.z_ > CHUNK_SIZE / 2)
        {
            auto offset = Vector3(0.0f, 0.0f, -CHUNK_SIZE);

            chunkPosXNegZ_->Remove();
            chunkCenterXNegZ_->Remove();
            chunkNegXNegZ_->Remove();

            chunkPosXNegZ_ = chunkPosXCenterZ_;
            chunkCenterXNegZ_ = chunkCenterXCenterZ_;
            chunkNegXNegZ_ = chunkNegXCenterZ_;

            OffsetNode(chunkPosXNegZ_, offset);
            OffsetNode(chunkCenterXNegZ_, offset);
            OffsetNode(chunkNegXNegZ_, offset);

            chunkPosXCenterZ_ = chunkPosXPosZ_;
            chunkCenterXCenterZ_ = chunkCenterXPosZ_;
            chunkNegXCenterZ_ = chunkNegXPosZ_;

            OffsetNode(chunkPosXCenterZ_, offset);
            OffsetNode(chunkCenterXCenterZ_, offset);
            OffsetNode(chunkNegXCenterZ_, offset);

            chunkPosXPosZ_ = GenerateChunk();
            chunkCenterXPosZ_ = GenerateChunk();
            chunkNegXPosZ_ = GenerateChunk();

            scene_->AddChild(chunkPosXPosZ_);
            scene_->AddChild(chunkCenterXPosZ_);
            scene_->AddChild(chunkNegXPosZ_);

            OffsetNode(chunkPosXPosZ_, -offset + Vector3(CHUNK_SIZE, 0.0f, 0.0f));
            OffsetNode(chunkCenterXPosZ_, -offset);
            OffsetNode(chunkNegXPosZ_, -offset + Vector3(-CHUNK_SIZE, 0.0f, 0.0f));

            OffsetNode(cameraNode_, offset);
        }

        if (cameraPos.z_ < -CHUNK_SIZE / 2)
        {
            auto offset = Vector3(0.0f, 0.0f, CHUNK_SIZE);

            chunkPosXPosZ_->Remove();
            chunkCenterXPosZ_->Remove();
            chunkNegXPosZ_->Remove();

            chunkPosXPosZ_ = chunkPosXCenterZ_;
            chunkCenterXPosZ_ = chunkCenterXCenterZ_;
            chunkNegXPosZ_ = chunkNegXCenterZ_;

            OffsetNode(chunkPosXPosZ_, offset);
            OffsetNode(chunkCenterXPosZ_, offset);
            OffsetNode(chunkNegXPosZ_, offset);

            chunkPosXCenterZ_ = chunkPosXNegZ_;
            chunkCenterXCenterZ_ = chunkCenterXNegZ_;
            chunkNegXCenterZ_ = chunkNegXNegZ_;

            OffsetNode(chunkPosXCenterZ_, offset);
            OffsetNode(chunkCenterXCenterZ_, offset);
            OffsetNode(chunkNegXCenterZ_, offset);

            chunkPosXNegZ_ = GenerateChunk();
            chunkCenterXNegZ_ = GenerateChunk();
            chunkNegXNegZ_ = GenerateChunk();

            scene_->AddChild(chunkPosXNegZ_);
            scene_->AddChild(chunkCenterXNegZ_);
            scene_->AddChild(chunkNegXNegZ_);

            OffsetNode(chunkPosXNegZ_, -offset + Vector3(CHUNK_SIZE, 0.0f, 0.0f));
            OffsetNode(chunkCenterXNegZ_, -offset);
            OffsetNode(chunkNegXNegZ_, -offset + Vector3(-CHUNK_SIZE, 0.0f, 0.0f));

            OffsetNode(cameraNode_, offset);
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
