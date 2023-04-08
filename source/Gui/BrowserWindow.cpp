#include "BrowserWindow.h"

#include <boost/algorithm/string/join.hpp>
#include <imgui.h>

#include "Fonts/IconsFontAwesome5.h"

#include "Base/Resources.h"
#include "Base/StringHelper.h"
#include "EngineInterface/Serializer.h"
#include "EngineInterface/SimulationController.h"

#include "AlienImGui.h"
#include "GlobalSettings.h"
#include "StyleRepository.h"
#include "RemoteSimulationDataParser.h"
#include "NetworkController.h"
#include "StatisticsWindow.h"
#include "Viewport.h"
#include "TemporalControlWindow.h"
#include "MessageDialog.h"
#include "LoginDialog.h"
#include "UploadSimulationDialog.h"
#include "DelayedExecutionController.h"
#include "OverlayMessageController.h"

_BrowserWindow::_BrowserWindow(
    SimulationController const& simController,
    NetworkController const& networkController,
    StatisticsWindow const& statisticsWindow,
    Viewport const& viewport,
    TemporalControlWindow const& temporalControlWindow)
    : _AlienWindow("Browser", "windows.browser", true)
    , _simController(simController)
    , _networkController(networkController)
    , _statisticsWindow(statisticsWindow)
    , _viewport(viewport)
    , _temporalControlWindow(temporalControlWindow)
{
     refreshIntern(true);
}

_BrowserWindow::~_BrowserWindow()
{
    _on = false;
}

void _BrowserWindow::registerCyclicReferences(LoginDialogWeakPtr const& loginDialog, UploadSimulationDialogWeakPtr const& uploadSimulationDialog)
{
    _loginDialog = loginDialog;
    _uploadSimulationDialog = uploadSimulationDialog;
}

void _BrowserWindow::onRefresh()
{
    refreshIntern(false);
}

void _BrowserWindow::refreshIntern(bool firstTimeStartup)
{
    try {
        if (!_networkController->getSimulationDataList(_remoteSimulationDatas, !firstTimeStartup)) {
            if (!firstTimeStartup) {
                MessageDialog::getInstance().show("Error", "Failed to retrieve browser data.");
            }
        }
        _filteredRemoteSimulationDatas = _remoteSimulationDatas;

        if (_networkController->getLoggedInUserName()) {
            std::vector<std::string> likedIds;
            if (!_networkController->getLikedSimulationIdList(likedIds)) {
                MessageDialog::getInstance().show("Error", "Failed to retrieve browser data.");
            }
            _likedIds = std::unordered_set<std::string>(likedIds.begin(), likedIds.end());
        } else {
            _likedIds.clear();
        }

        sortTable();
    } catch (std::exception const& e) {
        if (!firstTimeStartup) {
            MessageDialog::getInstance().show("Error", e.what());
        }
    }
}

void _BrowserWindow::processIntern()
{
    processToolbar();
    processTable();
    processStatus();
    processFilter();
    if(_scheduleRefresh) {
        onRefresh();
        _scheduleRefresh = false;
    }
}

void _BrowserWindow::processToolbar()
{
    if (AlienImGui::ToolbarButton(ICON_FA_SYNC)) {
        onRefresh();
    }
    AlienImGui::Tooltip("Refresh");

    ImGui::SameLine();
    ImGui::BeginDisabled(_networkController->getLoggedInUserName().has_value());
    if (AlienImGui::ToolbarButton(ICON_FA_SIGN_IN_ALT)) {
        if (auto loginDialog = _loginDialog.lock()) {
            loginDialog->show();
        }
    }
    ImGui::EndDisabled();
    AlienImGui::Tooltip("Login or register");

    ImGui::SameLine();
    ImGui::BeginDisabled(!_networkController->getLoggedInUserName());
    if (AlienImGui::ToolbarButton(ICON_FA_SIGN_OUT_ALT)) {
        if (auto loginDialog = _loginDialog.lock()) {
            _networkController->logout();
            onRefresh();
        }
    }
    ImGui::EndDisabled();
    AlienImGui::Tooltip("Logout");

    ImGui::SameLine();
    AlienImGui::ToolbarSeparator();

    ImGui::SameLine();
    ImGui::BeginDisabled(!_networkController->getLoggedInUserName());
    if (AlienImGui::ToolbarButton(ICON_FA_UPLOAD)) {
        if (auto uploadSimulationDialog = _uploadSimulationDialog.lock()) {
            uploadSimulationDialog->show();
        }
    }
    ImGui::EndDisabled();
    AlienImGui::Tooltip("Upload simulation");
    AlienImGui::Separator();
}

void _BrowserWindow::processTable()
{
    auto styleRepository = StyleRepository::getInstance();
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable
        | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("Browser", 12, flags, ImVec2(0, ImGui::GetContentRegionAvail().y - styleRepository.contentScale(63.0f)), 0.0f)) {
        ImGui::TableSetupColumn(
            "Actions", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_Actions);
        ImGui::TableSetupColumn(
            "Timestamp",
            ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending,
            0.0f,
            RemoteSimulationDataColumnId_Timestamp);
        ImGui::TableSetupColumn(
            "User name",
            ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed,
            styleRepository.contentScale(120.0f),
            RemoteSimulationDataColumnId_UserName);
        ImGui::TableSetupColumn(
            "Simulation name",
            ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed,
            styleRepository.contentScale(135.0f),
            RemoteSimulationDataColumnId_SimulationName);
        ImGui::TableSetupColumn(
            "Description",
            ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed,
            styleRepository.contentScale(120.0f),
            RemoteSimulationDataColumnId_Description);
        ImGui::TableSetupColumn("Stars", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_Likes);
        ImGui::TableSetupColumn("Downloads", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_NumDownloads);
        ImGui::TableSetupColumn("Width", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_Width);
        ImGui::TableSetupColumn("Height", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_Height);
        ImGui::TableSetupColumn("Objects", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_Particles);
        ImGui::TableSetupColumn("File size", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_FileSize);
        ImGui::TableSetupColumn("Version", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, RemoteSimulationDataColumnId_Version);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        //sort our data if sort specs have been changed!
        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
            if (sortSpecs->SpecsDirty || _scheduleSort) {
                if (_filteredRemoteSimulationDatas.size() > 1) {
                    std::sort(_filteredRemoteSimulationDatas.begin(), _filteredRemoteSimulationDatas.end(), [&](auto const& left, auto const& right) {
                        return RemoteSimulationData::compare(&left, &right, sortSpecs) < 0;
                    });
                }
                sortSpecs->SpecsDirty = false;
            }
        }

        ImGuiListClipper clipper;
        clipper.Begin(_filteredRemoteSimulationDatas.size());
        while (clipper.Step())
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {

                RemoteSimulationData* item = &_filteredRemoteSimulationDatas[row];

                ImGui::PushID(row);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                if (ImGui::Button(ICON_FA_DOWNLOAD)) {
                    onDownloadSimulation(item);
                }
                AlienImGui::Tooltip("Download");

                ImGui::SameLine();
                auto liked = isLiked(item->id);
                if (liked) {
                    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)Const::LikeTextColor);
                }
                if (ImGui::Button(ICON_FA_STAR)) {
                    if (_networkController->getLoggedInUserName()) {
                        onToggleLike(*item);
                    } else {
                        _loginDialog.lock()->show();
                    }
                }
                AlienImGui::Tooltip("Give a star");

                if (liked) {
                    ImGui::PopStyleColor(1);
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(item->userName != _networkController->getLoggedInUserName().value_or(""));
                if (ImGui::Button(ICON_FA_TRASH)) {
                    onDeleteSimulation(item);
                }
                ImGui::EndDisabled();
                AlienImGui::Tooltip("Delete");

                ImGui::TableNextColumn();

                pushTextColor(*item);

                AlienImGui::Text(item->timestamp);
                ImGui::TableNextColumn();
                processShortenedText(item->userName);
                ImGui::TableNextColumn();
                processShortenedText(item->simName);
                ImGui::TableNextColumn();
                processShortenedText(item->description);
                ImGui::TableNextColumn();
                AlienImGui::Text(std::to_string(item->likes));
                if(item->likes > 0) {
                    ImGui::SameLine();
                    processDetailButton();
                    AlienImGui::Tooltip([&] { return getUserLikes(item->id); }, false);
                }
                ImGui::TableNextColumn();
                AlienImGui::Text(std::to_string(item->numDownloads));
                ImGui::TableNextColumn();
                AlienImGui::Text(std::to_string(item->width));
                ImGui::TableNextColumn();
                AlienImGui::Text(std::to_string(item->height));
                ImGui::TableNextColumn();
                AlienImGui::Text(StringHelper::format(item->particles / 1000) + " K");
                ImGui::TableNextColumn();
                AlienImGui::Text(StringHelper::format(item->contentSize / 1024) + " KB");
                ImGui::TableNextColumn();
                AlienImGui::Text(item->version);
                ImGui::PopID();

                ImGui::PopStyleColor();
            }
        ImGui::EndTable();
    }
}

void _BrowserWindow::processStatus()
{
    auto styleRepository = StyleRepository::getInstance();

    if (ImGui::BeginChild("##", ImVec2(0, styleRepository.contentScale(33.0f)), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)Const::MonospaceColor);
        std::string statusText;
        statusText += std::string(" " ICON_FA_INFO_CIRCLE " ");
        statusText += std::to_string(_remoteSimulationDatas.size()) + " simulations found";

        statusText += std::string("  " ICON_FA_INFO_CIRCLE " ");
        if (auto userName = _networkController->getLoggedInUserName()) {
            statusText += "Logged in as " + *userName + " @ " + _networkController->getServerAddress();// + ": ";
        } else {
            statusText += "Not logged in to " + _networkController->getServerAddress();// + ": ";
        }

        if (!_networkController->getLoggedInUserName()) {
            statusText += std::string("   " ICON_FA_INFO_CIRCLE " ");
            statusText += "In order to upload and rate simulations you need to log in.";
        }
        AlienImGui::Text(statusText);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

void _BrowserWindow::processFilter()
{
    if (AlienImGui::InputText(AlienImGui::InputTextParameters().name("Filter"), _filter)) {
        _filteredRemoteSimulationDatas.clear();
        for (auto const& entry : _remoteSimulationDatas) {
            if (entry.matchWithFilter(_filter)) {
                _filteredRemoteSimulationDatas.emplace_back(entry);
            }
        }
    }
}

void _BrowserWindow::processShortenedText(std::string const& text) {
    auto styleRepository = StyleRepository::getInstance();
    auto textSize = ImGui::CalcTextSize(text.c_str());
    auto needDetailButton = textSize.x > ImGui::GetContentRegionAvailWidth();
    auto cursorPos = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvailWidth() - styleRepository.contentScale(15.0f);
    AlienImGui::Text(text);
    if (needDetailButton) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(cursorPos);

        processDetailButton();
        AlienImGui::Tooltip(text.c_str(), false);
    }
}

bool _BrowserWindow::processDetailButton()
{
    auto color = Const::DetailButtonColor;
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(color.Value.x, color.Value.y, color.Value.z, h, s, v);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(h, s, v * 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(h, s, v * 0.4f));
    auto detailClicked = AlienImGui::Button("...");
    ImGui::PopStyleColor(2);
    return detailClicked;
}

void _BrowserWindow::processActivated()
{
    onRefresh();
}

void _BrowserWindow::sortTable()
{
    _scheduleSort = true;
}

void _BrowserWindow::onDownloadSimulation(RemoteSimulationData* remoteData)
{
    printOverlayMessage("Downloading ...");

    delayedExecution([=, this] {
        SerializedSimulation serializedSim;
        if (!_networkController->downloadSimulation(serializedSim.mainData, serializedSim.auxiliaryData, remoteData->id)) {
            MessageDialog::getInstance().show("Error", "Failed to download simulation.");
            return;
        }

        DeserializedSimulation deserializedSim;
        if (!Serializer::deserializeSimulationFromStrings(deserializedSim, serializedSim)) {
            MessageDialog::getInstance().show("Error", "Failed to load simulation. Your program version may not match.");
            return;
        }

        _simController->closeSimulation();
        _statisticsWindow->reset();

        _simController->newSimulation(
            deserializedSim.auxiliaryData.timestep, deserializedSim.auxiliaryData.generalSettings, deserializedSim.auxiliaryData.simulationParameters);
        _simController->setClusteredSimulationData(deserializedSim.mainData);
        _viewport->setCenterInWorldPos(deserializedSim.auxiliaryData.center);
        _viewport->setZoomFactor(deserializedSim.auxiliaryData.zoom);
        _temporalControlWindow->onSnapshot();
    });
}

void _BrowserWindow::onDeleteSimulation(RemoteSimulationData* remoteData)
{
    printOverlayMessage("Deleting ...");

    delayedExecution([remoteData = remoteData, this] {
        if (!_networkController->deleteSimulation(remoteData->id)) {
            MessageDialog::getInstance().show("Error", "Failed to delete simulation.");
            return;
        }
        _scheduleRefresh = true;
    });
}

void _BrowserWindow::onToggleLike(RemoteSimulationData& entry)
{
    auto findResult = _likedIds.find(entry.id);
    if (findResult != _likedIds.end()) {
        _likedIds.erase(findResult);
        --entry.likes;
    } else {
        _likedIds.insert(entry.id);
        ++entry.likes;
    }
    _userLikesByIdCache.erase(entry.id); //invalidate cache entry
    _networkController->toggleLikeSimulation(entry.id);
    sortTable();
}

bool _BrowserWindow::isLiked(std::string const& id)
{
    return _likedIds.find(id) != _likedIds.end();
}

std::string _BrowserWindow::getUserLikes(std::string const& id)
{
    std::set<std::string> userLikes;

    auto findResult = _userLikesByIdCache.find(id);
    if (findResult != _userLikesByIdCache.end()) {
        userLikes = findResult->second;
    } else {
        _networkController->getUserLikesForSimulation(userLikes, id);
        _userLikesByIdCache.emplace(id, userLikes);
    }

    return boost::algorithm::join(userLikes, ", ");
}

void _BrowserWindow::pushTextColor(RemoteSimulationData const& entry)
{
    bool versionCompatible = true;

    std::vector<std::string> versionParts;
    boost::split(versionParts, entry.version, boost::is_any_of("."));

    auto majorVersion = std::stoi(versionParts.at(0));
    if (majorVersion < 4) {
        versionCompatible = false;
    } else if (versionParts.size() == 5 && versionParts.at(3) == "alpha") {
        auto alphaVersion = std::stoi(versionParts.at(4));
        if (alphaVersion < 2) {
            versionCompatible = false;
        }
    }

    if (versionCompatible) {
        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 0.0f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.6f));
    }
}
