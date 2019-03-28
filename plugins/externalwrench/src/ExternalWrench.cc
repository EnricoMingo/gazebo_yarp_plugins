#include "ExternalWrench.hh"

int ExternalWrench::count = 0;

//Initializing wrench command
ExternalWrench::ExternalWrench()
{
    model_has_link = false;

    //srand(boost::lexical_cast<float>(std::time(NULL)));
    srand(rand()%100);
    color[0] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    color[1] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    color[2] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    color[3] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    wrenchPtr->link_name = "world";
    wrenchPtr->force.resize(3,0);
    wrenchPtr->torque.resize(3,0);
    wrenchPtr->duration = 0.0;

    count++;
    tick = yarp::os::Time::now();
    duration_done = false;
}

bool ExternalWrench::getLink()
{
    // Get the exact link with only link name instead of full_scoped_link_name
    links = this->model->GetLinks();
    for(int i=0; i < links.size(); i++)
    {
        std::string candidate_link_name = links[i]->GetScopedName();

        // hasEnding compare ending of the condidate model link name to the given link name, in order to be able to use unscoped names
        if(GazeboYarpPlugins::hasEnding(candidate_link_name, wrenchPtr->link_name))
        {
            link = links[i];
            break;
        }
    }

    if(!link)
     {
        yError()  << "externalWrench: " << wrenchPtr->link_name << " link is not found";
        return false;
    }

    model_has_link = true;
    return true;
}

void ExternalWrench::setVisual()
{
    //Wrench Visual
    m_node = transport::NodePtr(new gazebo::transport::Node());
    this->m_node->Init(model->GetWorld()->Name());
    m_visPub = this->m_node->Advertise<msgs::Visual>("~/visual",100);

    // Set the visual's name. This should be unique.
    std::string visual_name = "__" + wrenchPtr->link_name + "__CYLINDER_VISUAL__" + boost::lexical_cast<std::string>(count);
    m_visualMsg.set_name (visual_name);

    // Set the visual's parent. This visual will be attached to the parent
    m_visualMsg.set_parent_name(model->GetScopedName());

    // Create a cylinder
    msgs::Geometry *geomMsg = m_visualMsg.mutable_geometry();
    geomMsg->set_type(msgs::Geometry::CYLINDER);
    geomMsg->mutable_cylinder()->set_radius(0.0075);
    geomMsg->mutable_cylinder()->set_length(.30);

    // Don't cast shadows
    m_visualMsg.set_cast_shadows(false);
}

bool ExternalWrench::setWrench(physics::ModelPtr& _model,yarp::os::Bottle& cmd)
{
    model = _model;

    //get link name from command
    std::string linkName = cmd.get(0).asString();
    if (linkName.find_last_of("::") != 0) {
        wrenchPtr->link_name = linkName.substr(linkName.find_last_of("::")+1); //case of scoped link name
    }
    else
    {
        wrenchPtr->link_name = linkName; //case of unscoped link name
    }

    if(getLink())
    {
        setVisual();

        wrenchPtr->force[0]  =  cmd.get(1).asDouble();
        wrenchPtr->force[1]  =  cmd.get(2).asDouble();
        wrenchPtr->force[2]  =  cmd.get(3).asDouble();

        wrenchPtr->torque[0] = cmd.get(4).asDouble();
        wrenchPtr->torque[1] = cmd.get(5).asDouble();
        wrenchPtr->torque[2] = cmd.get(6).asDouble();

        wrenchPtr->duration  = cmd.get(7).asDouble();

        return true;
    }
    else return false;
}

void ExternalWrench::applyWrench()
{
    tock = yarp::os::Time::now();
    if((tock-tick) < wrenchPtr->duration)
    {
#if GAZEBO_MAJOR_VERSION >= 8
        ignition::math::Vector3d force (wrenchPtr->force[0], wrenchPtr->force[1], wrenchPtr->force[2]);
        ignition::math::Vector3d torque (wrenchPtr->torque[0], wrenchPtr->torque[1], wrenchPtr->torque[2]);

        link->AddForce(force);
        link->AddTorque(torque);

        ignition::math::Vector3d linkCoGPos = link->WorldCoGPose().Pos(); // Get link's COG position where wrench will be applied
        ignition::math::Vector3d newZ = force.Normalize(); // normalized force. I want the z axis of the cylinder's reference frame to coincide with my force vector
        ignition::math::Vector3d newX = newZ.Cross (ignition::math::Vector3d::UnitZ);
        ignition::math::Vector3d newY = newZ.Cross (newX);
        ignition::math::Matrix4d rotation = ignition::math::Matrix4d (newX[0],newY[0],newZ[0],0,newX[1],newY[1],newZ[1],0,newX[2],newY[2],newZ[2],0, 0, 0, 0, 1);
        ignition::math::Quaterniond forceOrientation = rotation.Rotation();
        ignition::math::Pose3d linkCoGPose (linkCoGPos - rotation*ignition::math::Vector3d ( 0,0,.15 ), forceOrientation);
#else
        math::Vector3d force (wrenchPtr->force[0], wrenchPtr->force[1], wrenchPtr->force[2]);
        math::Vector3d torque (wrenchPtr->torque[0], wrenchPtr->torque[1], wrenchPtr->torque[2]);

        link->AddForce(force);
        link->AddTorque(torque);

        math::Vector3d linkCoGPos = link->WorldCoGPose().Pos(); // Get link's COG position where wrench will be applied
        math::Vector3d newZ = force.Normalize(); // normalized force. I want the z axis of the cylinder's reference frame to coincide with my force vector
        math::Vector3d newX = newZ.Cross (ignition::math::Vector3d::UnitZ);
        math::Vector3d newY = newZ.Cross (newX);
        math::Matrix4d rotation = ignition::math::Matrix4d (newX[0],newY[0],newZ[0],0,newX[1],newY[1],newZ[1],0,newX[2],newY[2],newZ[2],0, 0, 0, 0, 1);
        math::Quaterniond forceOrientation = rotation.Rotation();
        math::Pose3d linkCoGPose (linkCoGPos - rotation*ignition::math::Vector3d ( 0,0,.15 ), forceOrientation);
#endif

#if GAZEBO_MAJOR_VERSION == 7
        msgs::Set(m_visualMsg.mutable_pose(), linkCoGPose.Ign());
#else
        msgs::Set(m_visualMsg.mutable_pose(), linkCoGPose);
#endif
#if GAZEBO_MAJOR_VERSION >= 9
        msgs::Set(m_visualMsg.mutable_material()->mutable_ambient(), ignition::math::Color(color[0],color[1],color[2],color[3]));
#else
        msgs::Set(m_visualMsg.mutable_material()->mutable_ambient(),common::Color(color[0],color[1],color[2],color[3]));
#endif
        m_visualMsg.set_visible(1);
        m_visPub->Publish(m_visualMsg);
    }
    else
    {
        m_visualMsg.set_visible(0);
        m_visualMsg.clear_geometry();
        m_visualMsg.clear_delete_me();
        m_visPub->Publish(m_visualMsg);
        duration_done = true;
    }
}

ExternalWrench::~ExternalWrench()
{
    count--;
}
