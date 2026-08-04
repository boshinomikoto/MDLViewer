#include "StaticSceneApp.h"

namespace urho3d = Urho3D;

/*
    If you load an object with a real XML material, the color change works fine.
    The bug was specific to the default object without a material — that is why
    we substitute a real material from CoreData instead of passing an empty string.
*/
urho3d::Node* StaticSceneApp::loadMDLObject(urho3d::ResourceCache* cache,
                                             urho3d::String pathToXML,
                                             urho3d::String pathToMDL)
{
    if (!cache) return nullptr;

    urho3d::Node* node = scene_->CreateChild("Object");
    urho3d::Vector3 spawnPos = defaultObjPos;
    if (pathToMDL == "Models/Box.mdl") spawnPos.y_ = 0.5f;
    node->SetPosition(spawnPos);

    auto* tObject = node->CreateComponent<urho3d::StaticModel>();
    tObject->SetModel(cache->GetResource<urho3d::Model>(pathToMDL));

    urho3d::Material* material = pathToXML.Empty()
        ? cache->GetResource<urho3d::Material>("Materials/DefaultGrey.xml")
        : cache->GetResource<urho3d::Material>(pathToXML);

    urho3d::SharedPtr<urho3d::Material> uniqueMat = material ? material->Clone() : nullptr;
    tObject->SetMaterial(uniqueMat);

    node->CreateComponent<urho3d::RigidBody>();
    auto* shape = node->CreateComponent<urho3d::CollisionShape>();
    shape->SetTriangleMesh(tObject->GetModel());

    SetNormalizedScale(node, 1.0f);
    return node;
}

void StaticSceneApp::SetNormalizedScale(urho3d::Node* node, float targetSize)
{
    auto* modelComponent = node->GetComponent<urho3d::StaticModel>();
    if (!modelComponent || !modelComponent->GetModel()) return;

    urho3d::BoundingBox box  = modelComponent->GetModel()->GetBoundingBox();
    urho3d::Vector3     size = box.Size();
    float maxDim = urho3d::Max(size.x_, urho3d::Max(size.y_, size.z_));

    if (maxDim > 0.0f)
        node->SetScale(targetSize / maxDim);
}

void StaticSceneApp::TakenOutFuncForDeletObj()
{
    if (!tNode_) return;
    unsigned i        = 0;
    bool     notexist = true;
    do
    {
        if (objects_[i].node == tNode_ && objects_[i].drawGeometry == true)
        {
            objects_.Erase(i);
            tNode_->Remove();
            tNode_ = nullptr;
            break;
        }
        ++i;
        if (i == objects_.Size()) notexist = false;
    } while (notexist);
}

urho3d::Node* StaticSceneApp::PickObject(float maxDistance)
{
    urho3d::Ray ray(cameraNode_->GetWorldPosition(), cameraNode_->GetWorldDirection());

    urho3d::Vector<urho3d::RayQueryResult> results;
    urho3d::RayOctreeQuery query(results, ray,
        urho3d::RAY_TRIANGLE, maxDistance,
        urho3d::DrawableTypes::Geometry);

    scene_->GetComponent<urho3d::Octree>()->RaycastSingle(query);
    return results.Empty() ? nullptr : results[0].drawable_->GetNode();
}
