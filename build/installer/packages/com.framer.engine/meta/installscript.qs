
function Component()
{
    //installer.addWizardPage( component, "ErrorPage", QInstaller.ReadyForInstallation );
	//component.userInterface( "ErrorPage" ).complete = false;
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
	        component.addOperation("CreateShortcut", "@TargetDir@/bin/rmEd_x64.exe", "@DesktopDir@/Render Master.lnk", "iconPath=@TargetDir@/resources/editor/rm.ico");
			component.addOperation("CreateShortcut", "@TargetDir@/bin/rmEd_x64.exe", "@StartMenuDir@/Render Master.lnk", "iconPath=@TargetDir@/resources/editor/rm.ico");
			component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/Render Master Uninstall.lnk", "iconPath=@TargetDir@/resources/editor/rm.ico");
	}
}
