#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>

#include <argparse/argparse.hpp>
#include <opencv2/opencv.hpp>
#include <types/frame.hpp>
#include <utils/draw_utils.hpp>

#include <tracking/factory.hpp>

namespace fs = std::filesystem;

struct SequenceInfo
{
    std::string imDir;
    double frameRate{};
    int imWidth{};
    int imHeight{};
};

SequenceInfo parseSequenceInfo(const fs::path &iniPath)
{
    std::ifstream file(iniPath);
    if (!file)
    {
        std::println(std::cerr, "INI file not found: {}", iniPath.string());
        return {};
    }

    SequenceInfo info;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == ';')
            continue;

        auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        auto trim = [](std::string &s)
        {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };
        trim(key);
        trim(val);

        if (key == "imDir")
            info.imDir = val;
        else if (key == "frameRate")
            info.frameRate = std::stod(val);
        else if (key == "imWidth")
            info.imWidth = std::stoi(val);
        else if (key == "imHeight")
            info.imHeight = std::stoi(val);
    }
    return info;
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser parser("mot");
    parser.add_description("Multi Object Tracker");
    parser.add_argument("-i", "--input").required().help("Path to MOT sequence folder");
    parser.add_argument("-c", "--config").required().help("Path to tracker config.json");
    parser.add_argument("-o", "--output").help("Path to results folder (if not provided, output to stdout)");
    parser.add_argument("--gt").flag().help("Use ground-truth detections");
    parser.add_argument("-d", "--display").flag().help("Display images");
    parser.add_argument("-s", "--save").flag().help("Save video into output folder");

    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::println(std::cerr, "{}", e.what());
        std::cerr << parser;
        return 1;
    }

    // Input
    fs::path seqPath(parser.get("--input"));
    std::string seqName = seqPath.stem().string();
    auto seqInfo = parseSequenceInfo(seqPath / "seqinfo.ini");

    bool gt = parser.get<bool>("--gt");
    fs::path inPath = seqPath / (gt ? "gt/gt.txt" : "det/det.txt");

    std::ifstream inFile(inPath);
    if (!inFile.is_open())
    {
        std::println(std::cerr, "Could not open input file: {}", inPath.string());
        return 1;
    }

    bool display = parser.get<bool>("--display");
    fs::path imgPath = seqPath / seqInfo.imDir;
    if (display && !fs::is_directory(imgPath))
    {
        std::println(std::cerr, "Image directory does not exist: {}", imgPath.string());
        return 1;
    }

    // Config
    auto tracker = TrackerFactory::create(parser.get("--config"));
    if (!tracker)
    {
        std::println(std::cerr, "Failed to create tracker");
        return 1;
    }

    // Output
    auto outputDir = parser.present<std::string>("--output");
    std::ofstream outFile;
    std::ostream &out = [&]() -> std::ostream &
    {
        if (outputDir)
        {
            fs::path outPath = fs::path(*outputDir) / (seqName + ".txt");
            fs::create_directories(outPath.parent_path());
            outFile.open(outPath);
            if (!outFile.is_open())
            {
                std::println(std::cerr, "Could not open output file: {}", outPath.string());
                exit(1);
            }
            return outFile;
        }
        return std::cout;
    }();

    // Video writer
    bool saveVideo = parser.get<bool>("--save");
    if (saveVideo && !outputDir)
    {
        std::println(std::cerr, "Error: --save requires --output to be specified");
        return 1;
    }

    cv::VideoWriter videoWriter;
    cv::Size imageSize(seqInfo.imWidth, seqInfo.imHeight);
    if (saveVideo)
    {
        fs::path savePath = fs::path(*outputDir) / (seqName + ".mp4");
        videoWriter.open(savePath.string(), cv::VideoWriter::fourcc('m', 'p', '4', 'v'), seqInfo.frameRate, imageSize, true);

        if (!videoWriter.isOpened())
        {
            std::println(std::cerr, "Could not open the output video file for write");
            return 1;
        }
    }

    // Main
    std::istringstream iss;
    std::string line;
    std::string next_line;

    Detection detection;

    std::vector<fs::path> imageFiles;
    for (const auto &entry : fs::directory_iterator(imgPath))
        imageFiles.push_back(entry.path());
    std::sort(imageFiles.begin(), imageFiles.end());

    std::vector<Detection> detections;

    for (const auto &path : imageFiles)
    {
        Frame frame(cv::imread(path.string()));
        detections.clear();

        // Read detections from file
        while (true)
        {
            if (!next_line.empty())
            {
                line = next_line;
                next_line.clear();
            }
            else if (!std::getline(inFile, line))
            {
                break;
            }
            iss.clear();
            iss.str(line);
            iss >> detection;

            if (detection.frame_id == frame.getId())
                detections.push_back(detection);
            else
            {
                next_line = line;
                break;
            }
        }

        // Process detections
        tracker->update(detections);
        for (const auto &det : detections)
            out << det << "\n";

        // Visualize results
        cv::Mat output = drawDetections(frame, detections, true, true);

        if (saveVideo)
            videoWriter.write(output);

        if (display)
        {
            cv::imshow("Multi Object Tracking", output);
            if (cv::waitKey(static_cast<int>(1000.0 / seqInfo.frameRate)) == 27)
                break;
        }
    }

    if (videoWriter.isOpened())
        videoWriter.release();

    if (outFile.is_open())
        outFile.close();

    cv::destroyAllWindows();

    return 0;
}
