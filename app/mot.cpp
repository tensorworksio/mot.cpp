#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include <tracking/tracker.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

void getSequenceInfo(const fs::path &iniFilePath, po::variables_map &vm)
{
    po::options_description config("Sequence info");
    config.add_options()("Sequence.name", po::value<std::string>(), "Sequence name")("Sequence.imDir", po::value<std::string>(), "Image directory")("Sequence.frameRate", po::value<double>(), "Frame rate")("Sequence.seqLength", po::value<int>(), "Sequence length")("Sequence.imWidth", po::value<int>(), "Image width")("Sequence.imHeight", po::value<int>(), "Image height")("Sequence.imExt", po::value<std::string>(), "Image extension");

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

    po::options_description desc(" options");
    desc.add_options()("help,h", "multi object tracker")("tracker,t", po::value<std::string>()->required())("path,p", po::value<std::string>()->required(), "Path to MOT sequence folder")("config,c", po::value<std::string>()->default_value(""), "Path to SORT config.json")("gt", po::bool_switch()->default_value(false), "Ground truth mode")("display", po::bool_switch()->default_value(false), "Display frames")("save", po::bool_switch()->default_value(false), "Save video");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    po::notify(vm);

    // Tracker config
    TrackerType type = getTrackerType(vm["tracker"].as<std::string>());
    auto tracker = TrackerFactory::create(type, vm["config"].as<std::string>());

    // Display
    bool display = vm["display"].as<bool>();

    // Ground truth
    bool gt = vm["gt"].as<bool>();

    // Video saver
    bool saveVideo = vm["save"].as<bool>();

    // MOT path
    fs::path path(vm["path"].as<std::string>());
    fs::path outPath = path / "out.txt";

    // Extract sequence info
    fs::path seqInfoPath = path / "seqinfo.ini";
    getSequenceInfo(seqInfoPath, vm);

    fs::path detPath = path / "det" / "det.txt";
    if (!fs::exists(detPath))
    {
        std::cerr << "Detection file does not exist: " << detPath.string() << std::endl;
        return 1;
    }
    fs::path gtPath = path / "gt" / "gt.txt";
    if (gt && !fs::exists(gtPath))
    {
        std::cerr << "Ground truth file does not exist: " << gtPath.string() << std::endl;
        return 1;
    }

    fs::path imgPath = path / vm["Sequence.imDir"].as<std::string>();
    if (display && !fs::is_directory(imgPath))
    {
        std::cerr << "Image directory does not exist: " << imgPath.string() << std::endl;
        return 1;
    }

    // Video writer
    cv::VideoWriter videoWriter;
    fs::path videoPath = path / "output.mp4";
    cv::Size imageSize(vm["Sequence.imWidth"].as<int>(), vm["Sequence.imHeight"].as<int>());
    double fps = vm["Sequence.frameRate"].as<double>();

    if (saveVideo)
    {
        videoWriter.open(videoPath.string(), cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, imageSize, true);
    }

    if (saveVideo && !videoWriter.isOpened())
    {
        std::cerr << "Could not open the output video file for write" << std::endl;
        return 1;
    }

    std::ifstream infile(gt ? gtPath.string() : detPath.string());
    std::ofstream outfile(outPath.string());

    std::istringstream iss;
    std::ostringstream oss;
    std::string line;
    std::string next_line;

    Detection detection;
    Frame frame;

    std::vector<fs::path> imageFiles;
    for (const auto &entry : fs::directory_iterator(imgPath))
    {
        imageFiles.push_back(entry.path());
    }

    std::sort(imageFiles.begin(), imageFiles.end());

    for (const auto &path : imageFiles)
    {
        frame.idx++;
        frame.image = cv::imread(path.string());
        frame.detections.clear();

        // Read detections from file
        while (true)
        {
            if (!next_line.empty())
            {
                line = next_line;
                next_line.clear();
            }
            else if (!std::getline(infile, line))
            {
                break;
            }
            iss.str(line);
            iss >> detection;

            iss.str("");
            iss.clear();

            if (detection.frame == frame.idx)
            {
                frame.detections.push_back(detection);
            }
            else
            {
                next_line = line;
                break;
            }
        }

        // Process frame
        tracker->process(frame);

        // Write detections to file
        for (const auto &detection : frame.detections)
        {
            outfile << detection << std::endl;
        }

        // Display frame with detections
        for (const auto &detection : frame.detections)
        {
            auto color = detection.getColor();
            cv::rectangle(frame.image, detection.bbox, color, 2);
            cv::putText(frame.image, std::to_string(detection.id), cv::Point(detection.bbox.x, detection.bbox.y), cv::FONT_HERSHEY_SIMPLEX, 1, color, 2);
        }

        // Write video
        if (saveVideo)
        {
            videoWriter.write(frame.image);
        }
        // Display
        if (display)
        {
            cv::imshow("Frame", frame.image);
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

    cv::destroyAllWindows();
    outfile.close();

    return 0;
}