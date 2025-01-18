#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <tracking/factory.hpp>

#ifdef ENABLE_REID
#include <models/reid/reid.hpp>
#endif

namespace fs = boost::filesystem;
namespace po = boost::program_options;

void getSequenceInfo(const fs::path &iniFilePath, po::variables_map &vm)
{
    po::options_description config("Sequence info");
    config.add_options()("Sequence.name", po::value<std::string>(), "Sequence name");
    config.add_options()("Sequence.imDir", po::value<std::string>(), "Image directory");
    config.add_options()("Sequence.frameRate", po::value<double>(), "Frame rate");
    config.add_options()("Sequence.seqLength", po::value<int>(), "Sequence length");
    config.add_options()("Sequence.imWidth", po::value<int>(), "Image width");
    config.add_options()("Sequence.imHeight", po::value<int>(), "Image height");
    config.add_options()("Sequence.imExt", po::value<std::string>(), "Image extension");

    std::ifstream iniFile(iniFilePath.string());
    if (!iniFile)
    {
        std::cerr << "INI file not found: " << iniFilePath.string() << std::endl;
        return;
    }

    po::store(po::parse_config_file(iniFile, config), vm);
    po::notify(vm);
}

int main(int argc, char **argv)
{

    po::options_description options("Program options");
    options.add_options()("help,h", "Multi Object Tracker");
    options.add_options()("input,i", po::value<std::string>()->required(), "Path to MOT sequence folder");
    options.add_options()("config,c", po::value<std::string>()->required(), "Path to tracker config.json");
    options.add_options()("output,o", po::value<std::string>(), "Path to results folder (if not provided, output to stdout)");

    options.add_options()("reid,r", po::bool_switch()->default_value(false), "Enable reid, requires reid section in config.json");
    options.add_options()("gt", po::bool_switch()->default_value(false), "Use ground-truth detections");
    options.add_options()("display,d", po::bool_switch()->default_value(false), "Display images");
    options.add_options()("save,s", po::bool_switch()->default_value(false), "Save video into output folder");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);

    if (vm.count("help"))
    {
        std::cout << options << std::endl;
        return 1;
    }

    po::notify(vm);

    // Input
    fs::path seqPath(vm["input"].as<std::string>());
    std::string seqName = seqPath.stem().string();
    fs::path seqInfoPath = seqPath / "seqinfo.ini";
    getSequenceInfo(seqInfoPath, vm);

    bool gt = vm["gt"].as<bool>();
    fs::path detPath = seqPath / "det" / "det.txt";
    fs::path gtPath = seqPath / "gt" / "gt.txt";
    fs::path inPath = gt ? gtPath : detPath;

    std::ifstream inFile(gt ? gtPath.string() : detPath.string());
    if (!inFile.is_open())
    {
        std::cerr << "Could not open input file: " << inPath.string() << std::endl;
        return 1;
    }

    bool display = vm["display"].as<bool>();
    fs::path imgPath = seqPath / vm["Sequence.imDir"].as<std::string>();
    if (display && !fs::is_directory(imgPath))
    {
        std::cerr << "Image directory does not exist: " << imgPath.string() << std::endl;
        return 1;
    }

    // Config
    fs::path configPath = fs::path(vm["config"].as<std::string>());
    std::string configName = configPath.stem().string();
    auto tracker = TrackerFactory::create(configPath.string());
    if (!tracker)
    {
        std::cerr << "Failed to create tracker" << std::endl;
        return 1;
    }

#ifdef ENABLE_REID
    bool reid = vm["reid"].as<bool>();
    std::unique_ptr<ReId> reidModel = nullptr;
    if (reid)
    {
        auto reidConfig = ReIdConfig::load(configPath.string(), "reid");
        reidModel = std::make_unique<ReId>(reidConfig);
    }
#endif

    // Output
    std::ofstream outFile;
    std::ostream &out = [&]() -> std::ostream &
    {
        if (vm.count("output"))
        {
            fs::path outPath = fs::path(vm["output"].as<std::string>()) / configName / (seqName + ".txt");
            fs::create_directories(outPath.parent_path());
            outFile.open(outPath.string());
            if (!outFile.is_open())
            {
                std::cerr << "Could not open output file: " << outPath << std::endl;
                exit(1);
            }
            return outFile;
        }
        return std::cout;
    }();

    // Video writer
    bool saveVideo = vm["save"].as<bool>();
    if (saveVideo && !vm.count("output"))
    {
        std::cerr << "Error: --save requires --output to be specified" << std::endl;
        return 1;
    }

    cv::VideoWriter videoWriter;
    double fps = vm["Sequence.frameRate"].as<double>();
    cv::Size imageSize(vm["Sequence.imWidth"].as<int>(), vm["Sequence.imHeight"].as<int>());
    if (saveVideo)
    {
        fs::path savePath = fs::path(vm["output"].as<std::string>()) / configName / (seqName + ".mp4");
        videoWriter.open(savePath.string(), cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, imageSize, true);

        if (!videoWriter.isOpened())
        {
            std::cerr << "Could not open the output video file for write" << std::endl;
            return 1;
        }
    }

    // Main
    std::istringstream iss;
    std::ostringstream oss;
    std::string line;
    std::string next_line;

    Detection detection;
    cv::Mat frame;

    std::vector<fs::path> imageFiles;
    for (const auto &entry : fs::directory_iterator(imgPath))
    {
        imageFiles.push_back(entry.path());
    }

    std::sort(imageFiles.begin(), imageFiles.end());

    int frameIdx = 0;
    std::vector<Detection> detections;

    for (const auto &path : imageFiles)
    {
        frameIdx++;
        frame = cv::imread(path.string());
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

            if (detection.frame == frameIdx)
            {
                detections.push_back(detection);
            }
            else
            {
                next_line = line;
                break;
            }
        }

// Extract features from detections
#ifdef ENABLE_REID
        if (reid)
        {
            for (auto &det : detections)
            {
                // Clamp bbox coordinates to frame boundaries
                cv::Rect safeBbox = det.bbox;
                safeBbox.x = std::max(0, std::min(frame.cols - 1, safeBbox.x));
                safeBbox.y = std::max(0, std::min(frame.rows - 1, safeBbox.y));
                safeBbox.width = std::min(frame.cols - safeBbox.x, safeBbox.width);
                safeBbox.height = std::min(frame.rows - safeBbox.y, safeBbox.height);

                // Only process if bbox has valid dimensions
                if (safeBbox.width > 0 && safeBbox.height > 0)
                {
                    cv::Mat roi = frame(safeBbox);
                    det.features = reidModel->process(roi);
                }
            }
        }
#endif

        // Process detections
        tracker->update(detections);
        for (const auto &det : detections)
        {
            out << det << std::endl;
        }

        // Visualize results
        for (const auto &det : detections)
        {
            cv::rectangle(frame, det.bbox, det.getColor(), 2);
            std::string label = det.class_name + " ID:" + std::to_string(det.id);
            cv::putText(frame, label,
                        cv::Point(det.bbox.x, det.bbox.y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, det.getColor(), 2);
        }

        // Write video
        if (saveVideo)
        {
            videoWriter.write(frame);
        }

        // Display
        if (display)
        {
            cv::imshow("Frame", frame);
        }
        if (cv::waitKey(1000 / fps) == 27)
        {
            break;
        }
    }

    if (videoWriter.isOpened())
    {
        videoWriter.release();
    }

    if (outFile.is_open())
    {
        outFile.close();
    }

    cv::destroyAllWindows();

    return 0;
}