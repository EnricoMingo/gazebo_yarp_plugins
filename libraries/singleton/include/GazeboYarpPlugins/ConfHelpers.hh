/*
 * Copyright (C) 2007-2013 Istituto Italiano di Tecnologia ADVR & iCub Facility
 * Authors: Enrico Mingo, Alessio Rocchi, Mirko Ferrati, Silvio Traversaro and Alessandro Settimi
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#ifndef GAZEBOYARP_CONFIGURATION_HELPERS_HH
#define GAZEBOYARP_CONFIGURATION_HELPERS_HH

#include <boost/shared_ptr.hpp>

#include <sdf/sdf_config.h>

#if SDF_MAJOR_VERSION >= 3

#include <sdf/Element.hh>

#else

namespace sdf {
   class Element;
   typedef boost::shared_ptr<Element> ElementPtr;
}

#endif


namespace gazebo
{
    namespace sensors {
        class Sensor;
        typedef boost::shared_ptr<Sensor> SensorPtr;
    }
    namespace physics {
        class Model;
        typedef boost::shared_ptr<Model> ModelPtr;
    }
}

namespace yarp {
    namespace os {
        class Property;
    }
}

namespace GazeboYarpPlugins {

/**
 * Load the configuration for a given model plugin,
 * and save the configuration in the plugin_parameters output object.
 * This involves calling addGazeboEnviromentalVariablesModel and then loading
 * the yarp configuration file specified in the sdf with the yarpConfigurationFile tag.
 */
bool loadConfigModelPlugin(gazebo::physics::ModelPtr _parent,
                           sdf::ElementPtr _sdf,
                           yarp::os::Property & plugin_parameters);

/**
 * Add some Gazebo specific "enviromental variables" that it is possible
 * to expand in .ini configuration file loaded using the ${variable} syntax,
 * for writing configuration files where port names and other parameters
 * depend on Gazebo names.
 *
 * This function add some model related "enviromental variables",
 * copying their values from some ModelPtr methods :
 *
 * |  Yarp parameter name       | ModelPtr method    |
 * |:--------------------------:|:------------------:|
 * | gazeboYarpPluginsRobotName | model->GetName()   |
 *
 * @return true if all went well, false otherwise
 */
bool addGazeboEnviromentalVariablesModel(gazebo::physics::ModelPtr _parent,
                                         sdf::ElementPtr _sdf,
                                         yarp::os::Property & plugin_parameters);

/**
 * Load the configuration for a given sensor plugin,
 * and save the configuration in the plugin_parameters output object.
 * This involves calling addGazeboEnviromentalVariablesSensor and then loading
 * the yarp configuration file specified in the sdf with the yarpConfigurationFile tag.
 */
bool loadConfigSensorPlugin(gazebo::sensors::SensorPtr _sensor,
                            sdf::ElementPtr _sdf,
                            yarp::os::Property & plugin_parameters);

/**
 * Add some Gazebo specific "enviromental variables" that it is possible
 * to expand in .ini configuration file loaded using the ${variable} syntax,
 * for writing configuration files where port names and other parameters
 * depend on Gazebo names.
 *
 * This function add some Sensor related "enviromental variables",
 * copying their values from some SensorPtr methods :
 *
 * |  Yarp parameter name        | SensorPtr method    |
 * |:---------------------------:|:-------------------:|
 * | gazeboYarpPluginsSensorName | sensor->GetName()   |
 * | gazeboYarpPluginsRobotName  | Model name as extracted from sensor->GetScopedName()  |
 *
 * @return true if all went well, false otherwise
 */
bool addGazeboEnviromentalVariablesSensor(gazebo::sensors::SensorPtr _sensor,
                                          sdf::ElementPtr _sdf,
                                          yarp::os::Property & plugin_parameters);
/**
 * Setup verbosity of a gazebo yarp plugin.
 *
 * The verbosity is turned on if:
 *  * a `verbose` parameters is found in the .ini configuration file
 *  or
 *  * gazebo::common::Console::GetQuiet() return false (this means
 *    that gazebo has been launched with --verbose option.
 *
 * If verbosity is turned off, yarp::NetworkBase::setVerbosity(-1) will be called.
 * Note that this disable all the yarp prints on all the plugins,
 * not just in this one.
 *
 * This function is usually called in the loadConfig** helper functions.
 *
 * @param plugin_parameters Property containing the parameters for this plugin.
 * @return true if verbosity was enabled, false otherwise.
 */
bool initGazeboYarpPluginVerbosity(const yarp::os::Property & plugin_parameters);



}

#endif  // GAZEBOYARP_CONFIGURATION_HELPERS_HH
