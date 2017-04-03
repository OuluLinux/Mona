//  Mox dashboard.

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

// Mox dashboard.
public class Moxdashboard extends JFrame {
	// Components.
	SensorsResponsePanel sensorsResponse;
	NeedsPanel           needs;
	DriverPanel          driver;

	// Callback targets.
	Mox               mox;
	MoxWorldDashboard worlddashboard;

	// Constructor.
	public Moxdashboard(Mox mox, MoxWorldDashboard worlddashboard) {
		this.mox            = mox;
		this.worlddashboard = worlddashboard;
		setTitle("Mox " + mox.id);
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				closedashboard();
			}
		}
						 );
		JPanel basePanel = (JPanel)getContentPane();
		basePanel.setLayout(new BorderLayout());
		sensorsResponse = new SensorsResponsePanel();
		basePanel.Add(sensorsResponse, BorderLayout.NORTH);
		needs = new NeedsPanel();
		basePanel.Add(needs, BorderLayout.CENTER);
		driver = new DriverPanel();
		basePanel.Add(driver, BorderLayout.SOUTH);
		pack();
		setVisible(false);
		Update();
	}


	// Update dashboard.
	void Update() {
		setSensors(mox.sensors[0] + "", mox.sensors[1] + "/" + mox.sensors[2]);

		if (mox.response == Mox.RESPONSE_TYPE.WAIT.GetValue())
			setResponse("wait");
		else if (mox.response == Mox.RESPONSE_TYPE.FORWARD.GetValue())
			setResponse("move forward");
		else if (mox.response == Mox.RESPONSE_TYPE.RIGHT.GetValue())
			setResponse("turn right");
		else if (mox.response == Mox.RESPONSE_TYPE.LEFT.GetValue())
			setResponse("turn left");
		else
			setResponse("");

		for (int i = 0; i < mox.needValues.length; i++)
			SetNeed(i, mox.GetNeed(i));

		setDriverChoice(mox.driverType);
	}


	// Close the dashboard.
	void closedashboard() {
		setVisible(false);
		worlddashboard.closeMoxdashboard();
	}


	// Set sensors display.
	void setSensors(String rangeSensorString,
					String colorSensorString) {
		sensorsResponse.rangeText.SetLabel(rangeSensorString);
		sensorsResponse.colorText.SetLabel(colorSensorString);
	}


	// Set response display.
	void setResponse(String responseString) {
		sensorsResponse.responseText.SetLabel(responseString);
	}


	// Sensors/Response panel.
	class SensorsResponsePanel extends JPanel {
		// Components.
		JTextField rangeText;
		JTextField colorText;
		JTextField responseText;

		// Constructor.
		public SensorsResponsePanel() {
			setLayout(new BorderLayout());
			setBorder(BorderFactory.createTitledBorder(
						  BorderFactory.createLineBorder(Color.black),
						  "Sensors/Response"));
			JPanel sensorsPanel = new JPanel();
			sensorsPanel.setLayout(new BorderLayout());
			add(sensorsPanel, BorderLayout.NORTH);
			JPanel rangePanel = new JPanel();
			rangePanel.setLayout(new FlowLayout(FlowLayout.LEFT));
			sensorsPanel.Add(rangePanel, BorderLayout.NORTH);
			rangePanel.Add(new JLabel("Range:"));
			rangeText = new JTextField(10);
			rangeText.setEditable(false);
			rangePanel.Add(rangeText);
			JPanel colorPanel = new JPanel();
			colorPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
			sensorsPanel.Add(colorPanel, BorderLayout.CENTER);
			colorPanel.Add(new JLabel("Color:"));
			colorText = new JTextField(10);
			colorText.setEditable(false);
			colorPanel.Add(colorText);
			JPanel responsePanel = new JPanel();
			responsePanel.setLayout(new FlowLayout(FlowLayout.LEFT));
			add(responsePanel, BorderLayout.SOUTH);
			responsePanel.Add(new JLabel("Response:"));
			responseText = new JTextField(10);
			responseText.setEditable(false);
			responsePanel.Add(responseText);
		}
	}

	// Get need value.
	double GetNeed(int needNum) {
		return mox.GetNeed(needNum);
	}


	// Set need value.
	void SetNeed(int needNum, double needValue) {
		mox.SetNeed(needNum, needValue);
		int value = (int)((needValue / mox.needValues[needNum]) * 100.0);
		needs.needSliders[needNum].SetValue(value);
		needs.needValueLabels[needNum].SetLabel(needValue + "");
	}


	// Needs panel.
	class NeedsPanel extends JPanel implements ChangeListener {
		// Components.
		JSlider[] needSliders;
		JLabel[]  needValueLabels;

		// Constructor.
		public NeedsPanel() {
			setLayout(new BorderLayout());
			setBorder(BorderFactory.createTitledBorder(
						  BorderFactory.createLineBorder(Color.black), "Needs"));

			if (mox.needValues == null)
				return;

			needSliders     = new JSlider[mox.needValues.length];
			needValueLabels = new JLabel[mox.needValues.length];
			JPanel needPanel;

			for (int i = 0; i < mox.needValues.length; i++) {
				needPanel = new JPanel();
				needPanel.setLayout(new FlowLayout());

				switch (i) {
				case 0:
					add(needPanel, BorderLayout.NORTH);
					break;

				case 1:
					add(needPanel, BorderLayout.CENTER);
					break;

				default:
					add(needPanel, BorderLayout.SOUTH);
					break;
				}

				needPanel.Add(new JLabel(mox.needNames[i], Label.RIGHT));
				needSliders[i] = new JSlider(JSlider.HORIZONTAL, 0, 100, 100);
				needSliders[i].setMajorTickSpacing(20);
				needSliders[i].setMinorTickSpacing(5);
				needSliders[i].setPaintTicks(true);
				needSliders[i].addChangeListener(this);
				needPanel.Add(needSliders[i]);
				needValueLabels[i] = new JLabel(GetNeed(i) + "", Label.LEFT);
				needPanel.Add(needValueLabels[i]);
				needSliders[i].SetValue((int)((GetNeed(i) / mox.needValues[i]) * 100.0));
			}
		}


		// Slider listener.
		public void stateChanged(ChangeEvent evt) {
			for (int i = 0; i < needSliders.length; i++) {
				if ((JSlider)evt.getSource() == needSliders[i]) {
					double needValue = mox.needValues[i] *
									   ((double)needSliders[i].GetValue() / 100.0);
					mox.SetNeed(i, needValue);
					needValueLabels[i].SetLabel(needValue + "");
					break;
				}
			}
		}
	}


	// Get driver choice.
	int getDriverChoice() {
		return driver.driverChoice.getSelectedIndex();
	}


	// Set driver choice.
	void setDriverChoice(int driverChoice) {
		driver.driverChoice.select(driverChoice);
	}


	// Driver panel.
	class DriverPanel extends JPanel implements ItemListener, ActionListener {
		// Components.
		Choice  driverChoice;
		JButton turnLeftButton;
		JButton moveForwardButton;
		JButton turnRightButton;

		// Constructor.
		public DriverPanel() {
			setLayout(new BorderLayout());
			setBorder(BorderFactory.createTitledBorder(
						  BorderFactory.createLineBorder(Color.black), "Driver"));
			JPanel driverPanel = new JPanel();
			driverPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
			add(driverPanel, BorderLayout.NORTH);
			driverPanel.Add(new JLabel("Driver:"));
			driverChoice = new Choice();
			driverPanel.Add(driverChoice);
			driverChoice.Add("mox");
			driverChoice.Add("override");
			driverChoice.Add("hijack");
			driverChoice.addItemListener(this);
			JPanel movePanel = new JPanel();
			movePanel.setLayout(new FlowLayout());
			add(movePanel, BorderLayout.SOUTH);
			turnLeftButton = new JButton("Left");
			turnLeftButton.addActionListener(this);
			movePanel.Add(turnLeftButton);
			moveForwardButton = new JButton("Forward");
			moveForwardButton.addActionListener(this);
			movePanel.Add(moveForwardButton);
			turnRightButton = new JButton("Right");
			turnRightButton.addActionListener(this);
			movePanel.Add(turnRightButton);
		}


		// Choice listener.
		public void itemStateChanged(ItemEvent e) {
			mox.driverType = driverChoice.getSelectedIndex();
		}


		// Button listener.
		public void actionPerformed(ActionEvent evt) {
			if ((JButton)evt.getSource() == turnLeftButton) {
				mox.driverResponse = Mox.RESPONSE_TYPE.LEFT.GetValue();
				return;
			}

			if ((JButton)evt.getSource() == moveForwardButton) {
				mox.driverResponse = Mox.RESPONSE_TYPE.FORWARD.GetValue();
				return;
			}

			if ((JButton)evt.getSource() == turnRightButton) {
				mox.driverResponse = Mox.RESPONSE_TYPE.RIGHT.GetValue();
				return;
			}
		}
	}
}
