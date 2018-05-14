/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __CONFIGSERVICE_H
#define __CONFIGSERVICE_H

#include "Variant.h"
#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "SharedPtr.h"
#include "ServiceCommon.h"

namespace Opde {

/** @brief config service
 */
class ConfigService : public ServiceImpl<ConfigService> {
public:
    typedef enum {
        GAME_TYPE_INVALID = 0,
        GAME_TYPE_T1,
        GAME_TYPE_T2,
        GAME_TYPE_SS2
    } GameType;

    ConfigService(ServiceManager *manager, const std::string &name);
    virtual ~ConfigService();

    /** Sets a description for the specified parameter. */
    void setParamDescription(const std::string &param,
                             const std::string &value);

    /** Set a parameter */
    void setParam(const std::string &param, const std::string &value);

    /** get a parameter
     * @param param The parameter name
     * @param dflt The Variant default value, if not found
     * @return The parameter value, or Empty Variant if the parameter was not
     * found */
    Variant getParam(const std::string &param,
                      const Variant &dflt = Variant());

    /** get a parameter
     * @param param The parameter name
     * @param tgt The Variant instance to fill if successful (will not
     * overwrite if no param found)
     * @return true if parameter was changed */
    bool getParam(const std::string &param, Variant &tgt);

    /** determine an existence of a parameter */
    bool hasParam(const std::string &param);

    /** Injects the settings from a specified cfg file.
     * This function uses PlatformService to obtain two possible config file
     * paths. It then loads global followed by local version of the config file
     * mentioned.
     * @note If ConfigPathOverride is set (via setConfigPathOverride), then that
     * path is solely used to populate the configuration
     * @param cfgfile The name of the config file (without path)
     * @return true if config was loaded from at least one of the paths, false
     * otherwise */
    bool loadParams(const std::string &cfgfile);

    /** Sets an override for the config file loading paths. Used to enable
     * debugging without installation, etc.
     */
    void setConfigPathOverride(const std::string &cfgpath);

    /** returns the set game type. This should normally be the cam.cfg's game
     * dark/game shock, but we also need to differentiate t1/t2
     * @note Uses the game_type = t1/t2/ss2 config key */
    GameType getGameType();

    /** Language, as parsed from the install.cfg file
     * @todo Parse the install.cfg - now we read it from opde.cfg
     * @return Language name string (to be used with localized resource version
     * reader code)
     * */
    std::string getLanguage();

    /** Prepares a localized resource path out of given path.
     * What it does is that it takes the language setting, splits the origPath
     * into path and filename, and concatenates the language to the path
     * @param origPath The original path to process
     * @return The modified path with language string added to the path's end
     */
    std::string getLocalisedResourcePath(const std::string &origPath);

    /** Helper method that logs all the config parameters with descriptions to
     * LOG */
    void logAllParameters();

protected:
    /** initializes the service. Tries to load opde.cfg */
    bool init();
    void shutdown();

    /** Loads additional parameters from the specified file name */
    bool loadFromFile(const std::string &cfgfile);

private:
    typedef std::map<std::string, std::string> Parameters;

    Parameters mParameters;
    Parameters mConfigKeyDescriptions;

    PlatformServicePtr mPlatformService;

    /// If set, the hierarchical loading from global+local is not used, this
    /// path is used instead
    std::string mConfigPathOverride;
};

/// Factory for the ConfigService objects
class ConfigServiceFactory : public ServiceFactory {
public:
    ConfigServiceFactory();
    ~ConfigServiceFactory(){};

    /** Creates a ConfigService instance */
    Service *createInstance(ServiceManager *manager) override;

    const std::string &getName() override;

    const uint getMask() override;

    const size_t getSID() override;

private:
    static std::string mName;
};
} // namespace Opde

#endif
