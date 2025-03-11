#include "user_interface.h"
#include "network.h"
#include "includes.h"

std::string authToken = xorstr_("");
void user_interface::theme()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowMinSize = ImVec2(540, 370);
	Style.WindowBorderSize = 0;

	Style.ChildBorderSize = 0;
	Style.FrameBorderSize = 0;
	Style.WindowBorderSize = 0;

	Color[ImGuiCol_WindowBg] = ImColor(9, 9, 9, 255);

	Color[ImGuiCol_FrameBg] = ImColor(11, 11, 11, 255);
	Color[ImGuiCol_FrameBgActive] = ImColor(18, 17, 17, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(18, 17, 17, 255);

	Color[ImGuiCol_Button] = ImColor(14, 14, 14, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(17, 17, 17, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(20, 20, 20, 255);

	Color[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	Color[ImGuiCol_Separator] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripActive] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(30, 30, 30, 255);

	Color[ImGuiCol_ChildBg] = ImColor(0, 0, 0, 255);

	Color[ImGuiCol_ScrollbarBg] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);

	Color[ImGuiCol_Header] = ImColor(18, 17, 17, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(18, 17, 17, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(18, 17, 17, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);
}


char input_username[50] = "Nertigel";
char input_password[50] = "1234";
char input_redeem_key[50] = "";
bool showRedeemWindow = false;

ImFont* font_gram_ttf;
ImFont* font_gram2_ttf;
ImFont* font_gram3_ttf;
ImFont* font_gram_bold2_ttf;
ImFont* font_gram_bold_ttf;

ImFont* Font;

struct Notification {
    std::string message;
    float time_remaining;
};

std::vector<Notification> notifications;

void ShowNotifications() {
    float notification_duration = 3.0f;
    float padding = 3.0f;
    float notification_height = 40.0f;
    float notification_width = 250.0f;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 window_size = ImGui::GetWindowSize();
    float start_y = window_size.y - (notification_height + 10);
    float start_x = window_size.x - (notification_width + notification_height - 20);
    ImVec2 window_pos = ImGui::GetWindowPos(); // Get window position

    for (size_t i = 0; i < notifications.size();) {
        Notification& note = notifications[i];

        // Compute notification position relative to window position
        ImVec2 pos = ImVec2(window_pos.x + start_x, window_pos.y + start_y - (i * (notification_height + 10)));
        ImVec2 size = ImVec2(notification_width, notification_height);
        ImRect rect(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        // Expand rect for padding
        ImVec2 expandedMin = ImVec2(rect.Min.x, rect.Min.y);
        ImVec2 expandedMax = ImVec2(rect.Max.x, rect.Max.y);

        // Draw notification background
        window->DrawList->AddRectFilled(expandedMin, expandedMax, ImColor(11, 11, 11, 255), 5.f);
        window->DrawList->AddRect(expandedMin, expandedMax, ImColor(255, 255, 255, 35), 5.f);

        // Center text inside notification
        ImVec2 textSize = ImGui::CalcTextSize(note.message.c_str());
        ImVec2 textPos = ImVec2(rect.Min.x + (size.x - textSize.x) * 0.5f,
            rect.Min.y + (size.y - textSize.y) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        window->DrawList->AddText(textPos, IM_COL32(255, 255, 255, 255), note.message.c_str());
        ImGui::PopStyleColor();

        // Reduce notification time
        note.time_remaining -= ImGui::GetIO().DeltaTime;
        if (note.time_remaining <= 0) {
            notifications.erase(notifications.begin() + i);
        }
        else {
            ++i;
        }
    }
}
void DrawCenteredText(const char* text) {
    // Get the window size and text size
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 text_size = ImGui::CalcTextSize(text);

    // Calculate the horizontal position to center the text
    float x = (window_size.x - text_size.x) * 0.5f;  // Center horizontally

    // Set the cursor position to the calculated horizontal position (vertical stays the same)
    ImGui::SetCursorPosX(x);

    // Draw the text
    ImGui::Text("%s", text);
}

std::vector<Product> products = {
    //{"FiveM Spoofer", "Undetected", "Lifetime"},
    //{"FiveM Lua Executor", "Detected", "90 days"},
    //{"Roblox Executor", "Testing", "7 days"},
    //{"LLa test", "Unknown", "N/A"} // supplied by network.h
};

struct ChangeLog {
    std::string type;
    std::string description;
    std::string date;
};
std::vector<ChangeLog> latest_changes = {
    {xorstr_("Loader update"), xorstr_("Added authentication system"), xorstr_("09/03/2025")}
};

void user_interface::render_ui() {
    ImGui::NewFrame();
    if (user_interface::selected_tab == 1)
    {
        user_interface::theme();
        {
            ImGui::Begin(xorstr_("login_window"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            {
                ImVec2 CursorPos = ImGui::GetWindowPos();
                //ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(CursorPos.x, CursorPos.y + 200), ImVec2(CursorPos.x + 450, CursorPos.y + 800), ImVec2(CursorPos.x + 0, CursorPos.y + 800), ImColor(8, 8, 8, 255));

                ImGui::SetCursorPos(ImVec2(30, 320));

                ImGui::PushFont(Font);

                if (ImGui::ButtonNew(ICON_FA_SIGN_OUT, ImVec2(30, 30)))
                    exit(-1);

                ImGui::PopFont();

                ImGui::SetCursorPosY(50);
                ImGui::PushFont(font_gram3_ttf);
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 0, 0, 255));
                DrawCenteredText(xorstr_("DEMONIZED"));
                ImGui::PopFont();

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                DrawCenteredText(xorstr_("Welcome back, please login to continue"));
                ImGui::PopStyleColor();

                //ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
                //ImGui::PushFont(font_gram_ttf);
                //DrawCenteredText(xorstr_("Thank you for supporting our work!"));
                //ImGui::PopFont();
                //ImGui::PopStyleColor();

                ImGui::PushFont(font_gram3_ttf);
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(240, 240, 240, 255));

                int base_y_offset = 120;
                int base_y_add = 0;

                ImGui::SetCursorPos(ImVec2(100, base_y_offset + base_y_add));
                ImGui::Text(xorstr_("Username:"));
                base_y_add += 20;

                ImGui::SetCursorPos(ImVec2(100, base_y_offset + base_y_add));
                ImGui::InputTextNew(xorstr_("##iusername"), input_username, sizeof(input_username), ImVec2(350, 25));
                base_y_add += 40;

                ImGui::SetCursorPos(ImVec2(100, base_y_offset + base_y_add));
                ImGui::Text(xorstr_("Password:"));
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(345, base_y_offset + base_y_add));
                ImGui::PushStyleColor(ImGuiCol_TextHovered, IM_COL32(255, 55, 55, 255));
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(165, 165, 165, 255));
                ImGui::PushFont(font_gram_ttf);
                ImGui::ButtonExEm(xorstr_("Forgot password?"));
                ImGui::PopFont();
                base_y_add += 20;

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(240, 240, 240, 255));
                ImGui::PushFont(font_gram3_ttf);
                ImGui::SetCursorPos(ImVec2(100, base_y_offset + base_y_add));
                ImGui::InputTextNew(xorstr_("##ipassword"), input_password, sizeof(input_password), ImVec2(350, 25), ImGuiInputTextFlags_Password);
                ImGui::PopStyleColor();
                base_y_add += 50;

                ImGui::SetCursorPos(ImVec2(100, base_y_offset + base_y_add));
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

                if (ImGui::Button(xorstr_("Login"), ImVec2(350, 42)))
                {
                    std::string username(input_username);
                    std::string password(input_password);

                    if (!username.empty() && !password.empty())
                    {
                        authToken = network::get_auth_token(username, password);
                        if (!authToken.empty()) {
                            products = network::fetch_products(authToken);

                            notifications.push_back({ xorstr_("Login Successful!"), 3.0f });
                            user_interface::selected_tab = 2;
                        }
                        else
                        {
                            notifications.push_back({ xorstr_("Invalid Credentials!"), 3.0f });
                        }
                    }
                    else
                    {
                        notifications.push_back({ xorstr_("Please enter both fields!"), 3.0f });
                    }
                }

                ImGui::PopStyleColor();
                ImGui::PopFont();

                // ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(CursorPos.x , CursorPos.y + 450), ImVec2(CursorPos.x + 120, CursorPos.y + 800), ImColor(0, 0, 0, 255));

                ShowNotifications();

            }
            ImGui::End();
        }
    }
    else
    {
        user_interface::theme();
        ImGuiStyle& Style = ImGui::GetStyle();

        Style.WindowMinSize = ImVec2(200, 100);
        if (showRedeemWindow)
        {
            ImGui::Begin(xorstr_("redeem_key"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            {
                ImVec2 CursorPos = ImGui::GetWindowPos();

                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(5, 5, 5, 255));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(9, 9, 9, 255));

                ImGui::SetCursorPosY(10);
                ImGui::PushFont(font_gram3_ttf);
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                DrawCenteredText(xorstr_("Enter a key to redeem:"));

                ImGui::SetCursorPosX(20);
                ImGui::InputTextNew("##RenderKey", input_redeem_key, sizeof(input_redeem_key), ImVec2(170, 25));

                ImGui::PopFont();
                ImGui::PopStyleColor();

                ImGui::SetCursorPosX(20);
                if (ImGui::Button(xorstr_("Redeem")))
                {
                    std::string claim_key(input_redeem_key);
                    std::string response = network::try_redeem_key(authToken, claim_key);
                    if (response == xorstr_("Key claimed successfully!")) {
                        products = network::fetch_products(authToken);
                    }
                    notifications.push_back({ response, 3.0f });

                    showRedeemWindow = false;
                }
                ImGui::SameLine();
                if (ImGui::Button(xorstr_("Cancel")))
                    showRedeemWindow = false;
            }
            ImGui::End();
        }

        user_interface::theme();
        {
            ImGuiStyle& Style = ImGui::GetStyle();

            Style.WindowMinSize = ImVec2(580, 520);
            ImGui::Begin(xorstr_("second_window"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            {
                ImVec2 CursorPos = ImGui::GetWindowPos();

                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(5, 5, 5, 255));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(9, 9, 9, 255));

                //ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(CursorPos.x, CursorPos.y + 200), ImVec2(CursorPos.x + 450, CursorPos.y + 800), ImVec2(CursorPos.x + 0, CursorPos.y + 800), ImColor(8, 8, 8, 255));

                ImGui::PopStyleColor(2);
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(CursorPos.x, CursorPos.y), ImVec2(CursorPos.x + 70, CursorPos.y + 800), ImColor(8, 8, 8, 255));
                ImGui::GetWindowDrawList()->AddLine(ImVec2(CursorPos.x + 70, CursorPos.y), ImVec2(CursorPos.x + 70, CursorPos.y + 800), ImColor(255, 255, 255, 10));
                ImGui::PushFont(Font);

                ImGui::SetCursorPos(ImVec2(21, 40));
                if (ImGui::ButtonNew(ICON_FA_BARS, ImVec2(30, 30), user_interface::selected_sub_tab == 1))
                {
                    user_interface::selected_sub_tab = 1;
                    showRedeemWindow = false;
                }

                ImGui::SetCursorPos(ImVec2(21, 90));
                if (ImGui::ButtonNew(ICON_FA_CLOUD, ImVec2(30, 30), user_interface::selected_sub_tab == 2))
                {
                    user_interface::selected_sub_tab = 2;
                    showRedeemWindow = false;
                }

                ImGui::SetCursorPos(ImVec2(21, 140));
                if (ImGui::ButtonNew(ICON_FA_COGS, ImVec2(30, 30), user_interface::selected_sub_tab == 3))
                {
                    user_interface::selected_sub_tab = 3;
                    showRedeemWindow = false;
                }

                ImGui::SetCursorPos(ImVec2(21, 420));

                if (ImGui::ButtonNew(ICON_FA_KEY, ImVec2(30, 30), showRedeemWindow == true)) {
                    showRedeemWindow = !showRedeemWindow;
                }

                ImGui::SetCursorPos(ImVec2(21, 470));

                if (ImGui::ButtonNew(ICON_FA_SIGN_OUT, ImVec2(30, 30))) {
                    user_interface::selected_tab = 1;
                    showRedeemWindow = false;
                }

                ImGui::PopFont();

                std::string welcome_msg = xorstr_("Welcome back, ") + std::string(input_username);
                switch (user_interface::selected_sub_tab) {
                case 1:
                    ImGui::PushFont(font_gram_bold_ttf);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    ImGui::SetCursorPos(ImVec2(80, 10));
                    ImGui::Text(xorstr_("%s"), welcome_msg.c_str());
                    ImGui::PopFont();

                    ImGui::PushFont(font_gram_ttf);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
                    ImGui::SetCursorPos(ImVec2(80, 40));
                    ImGui::Text(xorstr_("Please choose an option from below"));
                    ImGui::PopFont();

                    ImGui::SetCursorPos(ImVec2(30, 30));

                    ImGui::GetWindowDrawList()->AddLine(ImVec2(CursorPos.x + 75, CursorPos.y + 65), ImVec2(CursorPos.x + 570, CursorPos.y + 65), ImColor(255, 255, 255, 10));

                    ImGui::SetCursorPos(ImVec2(80, 75));

                    for (const auto& product : products) {
                        ImVec2 childSize = ImVec2(490, 80);
                        ImVec2 cursor = ImGui::GetCursorScreenPos();
                        ImVec2 boxMin = cursor;
                        ImVec2 boxMax = ImVec2(cursor.x + childSize.x, cursor.y + childSize.y);

                        ImDrawList* drawList = ImGui::GetWindowDrawList();

                        // Background Layers (Matching the previous design)
                        drawList->AddRectFilled(boxMin, boxMax, IM_COL32(11, 11, 11, 255), 3.0f);  // Dark background
                        //drawList->AddRectFilled(boxMin, boxMax, IM_COL32(77, 77, 77, 120), 3.0f);  // Slight overlay
                        drawList->AddRect(boxMin, boxMax, IM_COL32(255, 255, 255, 30), 3.0f);      // Soft outline

                        ImGui::SetCursorScreenPos(cursor);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));

                        ImGui::BeginChild(product.name.c_str(), childSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
                        {
                            ImGui::PushFont(font_gram3_ttf);
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

                            ImGui::SetCursorPos(ImVec2(10, 10));
                            ImGui::Text(xorstr_("%s"), product.name.c_str());
                            ImGui::SetCursorPos(ImVec2(10, 30));
                            if (product.status == xorstr_("Undetected") || product.status == xorstr_("Online")) {
                                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255));
                            }
                            else if (product.status == xorstr_("Detected") || product.status == xorstr_("Offline")) {
                                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                            }
                            else if (product.status == xorstr_("Testing")) {
                                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 69, 0, 255));
                            }
                            else {
                                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(169, 169, 169, 255));
                            }
                            ImGui::Text(xorstr_("%s"), product.status.c_str());
                            ImGui::PopStyleColor();
                            ImGui::SetCursorPos(ImVec2(10, 50));
                            ImGui::Text(xorstr_("Expiration date: %s"), product.expiry.c_str());
                            ImGui::PopFont();

                            ImGui::SetCursorPos(ImVec2(430, 45));
                            if (ImGui::Button((xorstr_("Load##") + product.name).c_str(), ImVec2(50, 25))) {
                                if (product.status == xorstr_("Undetected") || product.status == xorstr_("Online")) {

                                }
                                else {
                                    notifications.push_back({ (xorstr_("Software is ") + product.status).c_str(), 3.0f });
                                }
                            }
                            ImGui::PopStyleColor();
                        }
                        ImGui::EndChild();

                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar();

                        ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + childSize.y + 10)); // Adjust for next element
                    }
                    break;
                case 2:
                    ImGui::PushFont(font_gram_bold_ttf);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    ImGui::SetCursorPos(ImVec2(80, 10));
                    ImGui::Text(xorstr_("Changelog"));
                    ImGui::PopFont();

                    ImGui::PushFont(font_gram_ttf);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
                    ImGui::SetCursorPos(ImVec2(80, 40));
                    ImGui::Text(xorstr_("Recent changes"));
                    ImGui::PopFont();

                    ImGui::GetWindowDrawList()->AddLine(ImVec2(CursorPos.x + 75, CursorPos.y + 65), ImVec2(CursorPos.x + 570, CursorPos.y + 65), ImColor(255, 255, 255, 10));

                    ImGui::SetCursorPos(ImVec2(80, 75));

                    for (const auto& changelog : latest_changes) {
                        ImVec2 childSize = ImVec2(490, 60);
                        ImVec2 cursor = ImGui::GetCursorScreenPos();
                        ImVec2 boxMin = cursor;
                        ImVec2 boxMax = ImVec2(cursor.x + childSize.x, cursor.y + childSize.y);

                        ImDrawList* drawList = ImGui::GetWindowDrawList();

                        // Background Layers (Matching the previous design)
                        drawList->AddRectFilled(boxMin, boxMax, IM_COL32(11, 11, 11, 255), 3.0f);  // Dark background
                        //drawList->AddRectFilled(boxMin, boxMax, IM_COL32(77, 77, 77, 120), 3.0f);  // Slight overlay
                        drawList->AddRect(boxMin, boxMax, IM_COL32(255, 255, 255, 30), 3.0f);      // Soft outline

                        ImGui::SetCursorScreenPos(cursor);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0)); // Transparent child background

                        ImGui::BeginChild(("##" + changelog.date).c_str(), childSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
                        {
                            ImGui::PushFont(font_gram3_ttf);
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 200));

                            ImGui::SetCursorPos(ImVec2(10, 10));
                            ImVec2 textSize = ImGui::CalcTextSize(changelog.type.c_str());
                            ImGui::Text(changelog.type.c_str());

                            ImGui::SetCursorPos(ImVec2((ImGui::GetCursorPosX() + textSize.x + 15), 10));

                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
                            ImGui::Text(xorstr_("%s"), changelog.date.c_str());
                            ImGui::PopStyleColor();

                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                            ImGui::SetCursorPos(ImVec2(10, 30));
                            ImGui::Text(xorstr_("%s"), changelog.description.c_str());
                            ImGui::PopStyleColor();

                            ImGui::PopStyleColor();
                        }
                        ImGui::EndChild();

                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar();

                        ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + childSize.y + 10)); // Adjust for next element
                    }
                    break;
                case 3:
                    ImGui::PushFont(font_gram_bold_ttf);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    ImGui::SetCursorPos(ImVec2(80, 10));
                    ImGui::Text(xorstr_("Settings"));
                    ImGui::PopFont();

                    ImGui::PushFont(font_gram_ttf);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
                    ImGui::SetCursorPos(ImVec2(80, 40));
                    ImGui::Text(xorstr_("Modify loader options to your liking"));
                    ImGui::PopFont();

                    ImGui::GetWindowDrawList()->AddLine(ImVec2(CursorPos.x + 75, CursorPos.y + 65), ImVec2(CursorPos.x + 570, CursorPos.y + 65), ImColor(255, 255, 255, 10));

                    ImGui::SetCursorPos(ImVec2(80, 75));

                    break;
                case 4:
                    break;
                }

                ShowNotifications();
            }
            ImGui::End();
        }
    }

    ImGui::EndFrame();
}